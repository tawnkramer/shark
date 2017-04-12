#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <zmq.h>
#include <czmq.h>
#include "zmq/zhelpers.h"
#include "joystick/joystick.hh"
#include "SharkConfig.h"
#include "json.h"
#include "config.h"

#define TJE_IMPLEMENTATION
#include "tiny_jpeg/tiny_jpeg.h"

#if ENABLE_RASPICAM
#include "raspicam/raspicam.h"
using namespace raspicam;
#endif


//this can be disabled to run on linux platforms w no i2c for testing camera, prediction code.
#if ENABLE_CAR
#include "mcqueen/car/Car.h"
#else
#warning "car disabled"
#endif

//if we need a usb camera support
#include "v4l_helper/capture_raw_frames.h"

#define MAX_MESSAGE_LEN 1024
#define MAX_STR_LEN 1024
#define MAX_PATH_LEN 1024

//Program run state
volatile bool programRunning = false;
volatile bool programExited = false;

//We can handle three different camera types.
enum CameraType
{
    Cam_Raspi,      //Raspicam 
    Cam_V4l,        //Any v4l camera, I'm using PS3 Eye
    Cam_PointGrey,  //A point grey camera using flycapture sdk 
};

//We can handle different NN backends
enum PreditionType
{
    Pred_Keras,     //We connect to a python process running keras
};

///////////////////////////////////////////////////////////////////////////////
//sends some message. uses the length of null terminated string to determine size.
//returns num of bytes sent.

size_t send_message(void* socket, const char* str_data)
{
   size_t size = zmq_send (socket, str_data, strlen (str_data), 0);
   return size;
}

///////////////////////////////////////////////////////////////////////////////
//sends some message. uses the specified length.
//returns num of bytes sent.

size_t send_message(void* socket, const void* data, size_t len)
{
   size_t size = zmq_send (socket, data, len, 0);
   return size;
}

///////////////////////////////////////////////////////////////////////////////
//poll for a message without blocking. Returns > 0 when we have data.

int recv_message(void* socket, char* buffer, const int maxSizeBuffer)
{
    int flags = ZMQ_DONTWAIT;

    int ret = zmq_recv (socket, buffer, maxSizeBuffer, flags);

    if(ret > 0 && ret < maxSizeBuffer)
    {
        buffer[ret] = 0;
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// A record of the last joystick axis input

struct AxisRecord
{
    AxisRecord() : steer(0), throttle(0) {}
    int steer;
    int throttle;
    uint64_t tick;
};

///////////////////////////////////////////////////////////////////////////////
// A record of the last button input, -1 means not set

struct ButtonRecord
{
    ButtonRecord() : button(-1), state(-1) {}

    int button;
    int state;
    uint64_t tick;   
};

///////////////////////////////////////////////////////////////////////////////
//raw image buffer bytes

struct ImageRecord
{
    ImageRecord() : image(NULL), maxImageLen(0) {} 

    ~ImageRecord() {
        if (image != NULL)
         delete[] image;
    }

    void InitImage(uint64_t len)
    {
        maxImageLen = len;
        image = new char[maxImageLen];
    }

    char* image;
    uint64_t maxImageLen;
    uint64_t tick;
};

///////////////////////////////////////////////////////////////////////////////
//A lock free ring buffer that allows a consumer and producer to write
//and read at once from different threads. The reader gets the latest record written.
//The writer is always advancing iregardless of the reader.

template<class Type, int Dim>
class RingBuffer
{
    public:

    RingBuffer()
    {
        iWriting = 0;
        iReading = -1;
    }

    //Write will deep copy your record into the ring buffer
    //and then advance the reading and writing heads
    void Write(const Type& record)
    {
        m_Buffer[iWriting] = record;
        iReading = iWriting;
        iWriting = (iWriting + 1) % Dim;
    }

    //Optionally, BeginWrite breaks the write down into a two step process.
    //This helps avoid the deep copy when we have lots of memory in our record.
    //For the image record this saves about 5% of cpu usage per copy.
    Type& BeginWrite()
    {
        return m_Buffer[iWriting];
    }

    //Step two of the two step write. This takes place after the copy is done.
    void FinishWrite()
    {
        iReading = iWriting;
        iWriting = (iWriting + 1) % Dim;
    }

    //this read makes a deep copy of the record for the user.
    bool Read(Type& record)
    {
        if(iReading > -1)
        {
            record = m_Buffer[iReading];
        }

        return iReading > -1;
    }

    int Size()
    {
        return Dim;
    }

    //this returns a reference to the record at the read head
    //For the image record this saves about 5% of cpu usage per copy over deep copy.   
    Type* ReadRef()
    {
        if(iReading > -1)
        {
            return &m_Buffer[iReading];
        }

        return NULL;
    }

    volatile int iWriting;
    volatile int iReading;

    Type m_Buffer[Dim];
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//Our ring buffer of axis inputs
RingBuffer<AxisRecord, 10> g_AxisInput;

//Ring buffer of button presses
RingBuffer<ButtonRecord, 10> g_ButtonInput;

//Ring buffer of axis inputs for predictions
RingBuffer<AxisRecord, 10> g_PredInput;

//Our ring buffer of images
RingBuffer<ImageRecord, 3> g_Images;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// set max image size

void InitImageRecordSize(Config& conf)
{
    //Init image dimensions
    int rows = conf.GetInt("row", 120);
    int cols = conf.GetInt("col", 160);
    int ch = conf.GetInt("ch", 3);

    uint64_t len = (rows * cols * ch);

    for(int iR = 0; iR < g_Images.Size(); iR++ )
        g_Images.m_Buffer[iR].InitImage(len);
}

///////////////////////////////////////////////////////////////////////////////
//Keep track of how much time elapsed per iteration

class Profiler
{

public:

    Profiler(const char* label, int iFrameToPrint)
    {
        m_iFrameToPrint = iFrameToPrint;
        m_iFrame = 0;
        m_start = s_clock();
        strncpy(m_label, label, MAX_STR_LEN);
        m_label[MAX_STR_LEN] = 0;
    }

    void Reset()
    {
        m_iFrame = 0;
        m_start = s_clock();
    }

    void OnFrameIter()
    {
        m_iFrame++;

        if(m_iFrame == m_iFrameToPrint)
        {
            double t = get_sec_diff(s_clock(), m_start);
            printf("%s:\t %d fps\n", m_label, (int)((float)m_iFrame / t));
            m_start = s_clock();
            m_iFrame = 0;
        }
    }

protected:

    int m_iFrame;
    int m_iFrameToPrint;
    char m_label[MAX_STR_LEN];
    uint64_t m_start;
};


///////////////////////////////////////////////////////////////////////////////
//Sizaxis spews orientation data. Some may like it. But some may
//want to filter it out. Does this axisId match our list of filters?

bool doIgnoreAxis(int* ignoreIds, int ignoreCount, int axisId)
{
    for(int iAxis = 0; iAxis < ignoreCount; iAxis++)
        if(ignoreIds[iAxis] == axisId)
            return true;

    return false;
}


///////////////////////////////////////////////////////////////////////////////
//LED status light

#if ENABLE_CAR
Gpio* pGpio = NULL;
#endif

int status_pin = 16;
uint64_t last_change = 0;
bool led_on = false;
void led_status(bool onOff);

void blink_led_status(float rate)
{
    float time_since_change = get_sec_diff(s_clock(), last_change);
    
    if(time_since_change > rate)
    {  
        led_on = !led_on;
        led_status(led_on);
        last_change = s_clock();
    }
}

void led_status(bool onOff)
{
#if ENABLE_CAR

    if(pGpio == NULL)
        pGpio = new Gpio(2);

    Pin* pPin = pGpio->getPin(status_pin);

    if(pPin != NULL)
    {
        if(onOff == false)
            pPin->setValue(0);
        else if(onOff == true)
            pPin->setValue(1);
    }

#endif
}

///////////////////////////////////////////////////////////////////////////////
//publish axis and button data from a joystick

void* ProcessJoyStick(void* args)
{
    Config* conf = (Config*)args;

    Joystick joystick;

    bool verbose = conf->GetInt("debug_test_js", 0) == 1;

    const char* js_path = conf->GetStr("js_path", "/dev/input/js0");

    //Wait for joystick to connect. It may be connected via Bluetooth and take a moment
    //to arrive. Once connected, we don't yet handle disconnect/reconnect.
    while (!joystick.isFound() && programRunning)
    {
        // Restrict rate
        usleep(1000);
        joystick.openPath(js_path);
    }

    if(verbose)
        printf("joystick is found on: %s\n", js_path);

    //The PS3 sixaxis controller spews constant orientation data
    //on these three axis channels. Suppressing this saves time downstream.
    int ignoreAxis[] = { 25, 24, 23 };
    const int numIgnoreAxis = sizeof(ignoreAxis) / sizeof(int);

    //PS3 left analog left and right is axis 0
    int axisSteer = conf->GetInt("js_axis_steer", 0);

    //PS3 right analog up and down is axis 3
    int axisThrottle = conf->GetInt("js_axis_throttle", 3);

    //We reuse this AxisRecord to keep and publish the current state.
    AxisRecord record;

    //A profiler lets us know every N frames how we are doing.
    //We update this quite frequently 900+ FPS
    Profiler profile("Jystck", 3000);

    //continue to loop.
    while (programRunning)
    {
        // Restrict rate
        usleep(1000);

        // Attempt to sample an event from the joystick
        JoystickEvent event;
       
        if (joystick.sample(&event))
        {
            if (event.isButton())
            {
                ButtonRecord r;
                r.button = event.number;
                r.state = event.value;
                r.tick = clock();
                g_ButtonInput.Write(r);
                printf("Button %u is %s\n", event.number, event.value == 0 ? "up" : "down");
            }
            else if (event.isAxis() && 
                !doIgnoreAxis(ignoreAxis, numIgnoreAxis, event.number))
            {
                if(event.number == axisSteer)
                {
                    if(verbose)
                        printf("steering input: %d\n", event.value);

                    record.steer = event.value;
                    record.tick = clock();
                    g_AxisInput.Write(record);
                }
                else if(event.number == axisThrottle)
                {
                    if(verbose)
                        printf("throttle input: %d\n", event.value);

                    //we reverse the throttle so Up is forward.
                    record.throttle = event.value * -1;
                    record.tick = clock();
                    g_AxisInput.Write(record);
                }
            }
        }

        profile.OnFrameIter();
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

struct Pixel
{
    unsigned char r, g, b;
};

///////////////////////////////////////////////////////////////////////////////
// Capture frames from a raspicam

void* ProcessRaspiCamera(void* args)
{
#if ENABLE_RASPICAM
    Config* conf = (Config*)args;

    int width = conf->GetInt("raspi_camera_cap_width", 160);
    int height = conf->GetInt("raspi_camera_cap_height", 120);

    raspicam::RaspiCam* cam = new raspicam::RaspiCam();

    cam->setWidth(width);
    cam->setHeight(height);
    
    cam->setBrightness(50);
    cam->setSharpness(0);
    cam->setContrast(0);
    cam->setSaturation(0);
    cam->setShutterSpeed(0);
    cam->setISO(400);
    
    cam->setExposureCompensation(0);
    //cam->setExposure(RASPICAM_EXPOSURE_AUTO);
    //cam->setAWB(RASPICAM_AWB_AUTO);
    
    cam->setAWB_RB(1.0f, 1.0f);
    cam->setFormat(RASPICAM_FORMAT_RGB);
    
    //////////////////////////////////////////////////
    /// Source lib HACK!!!
    //cam->setFrameRate(0);
    //for some reason the raspicam setFrameRate isnt' working. I changed
    //the framerate in the library.
    //raspicam/src/private/private_impl.cpp
    //change: State.framerate            = 10;
    //to:     State.framerate            = 60;
    //
    // make this change and recompile raspicam.
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////

    //Init image dimensions
    const int rows = conf->GetInt("row", 120);
    const int cols = conf->GetInt("col", 160);
    const int ch = conf->GetInt("ch", 3);
    const int max_image_len = rows * cols * ch;

    //we just need a small portion of the frame.
    cam->setCaptureSize(cols, rows);

    if (!cam->open(true))
    {
        printf("open raspberrypi camera failed.\n");
        delete cam;
        return NULL;
    }    


    unsigned char* imageData = NULL;
    size_t imageSize = 0;
    Profiler profile("Camera", 100);

    while (programRunning)
    {
       // Restrict rate
       usleep(1000);

        // Attempt to sample frame from the camera
       if(cam->grab())
       {
            imageData = cam->getImageBufferData();
            imageSize = cam->getImageBufferSize();

            if(imageSize == max_image_len)
            {
                //desination frame
                ImageRecord& img = g_Images.BeginWrite();                
            
                //red and blue are swapped. should we do it here? seems to be fast enough.
                //we will do the copy to destination in the same operation.
                Pixel* p = (Pixel*)imageData;
                Pixel* end = p + imageSize / ch;
                Pixel* d = (Pixel*)&img.image[0];

                while(p < end)
                {
                    d->r = p->b;
                    d->g = p->g;
                    d->b = p->r;

                    d++;
                    p++;
                }

                //stamp image with time stamp
                img.tick = clock();
                
                //advance the read head
                g_Images.FinishWrite();
            }
            else
            {
                //printf("wrong image size.\n");
            }

            profile.OnFrameIter();
       }
    }

    cam->release();
    delete cam;

#else
    
    printf("Raspicam support not enabled.\n");

#endif //ENABLE_RASPICAM

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Capture frames from a video for linux device ( USB or whatnot.. )

Profiler g_v4l_profile("V4l", 900);

struct CaptureSettings
{
    int capture_width;
    int capture_height;
    int dest_width;
    int dest_height;
};

/// Expects a YUYV image, processes to RGB and then adds to
/// the image queue for others to use.
void process_image(const void *p, int size, void* userData)
{
    CaptureSettings* cs = (CaptureSettings*)userData;
    
    //we are expecting a YUYV frame 320, 240.
    const int width = cs->capture_width;
    const int height = cs->capture_height;
    const int d = 2;

    //we are capturing this portion of the image. In this version
    //we are getting exactly 1/4 of the information, or half
    //native resolution in each dim.
    int destw = cs->dest_width;
    int desth = cs->dest_height;

    //this version assumes an evenly scaled version of w and heigh
    if((width / 2 != destw))
        return;

    if((height / 2 != desth))
        return;

    if(size != (width * height * d))
    {
        //printf("wrong image size. Expected: %d, got: %d\n", (width * height * d), size);
        return;
    }

    int y;
    int cr;
    int cb;

    double r;
    double g;
    double b;

    //get a reference to the next record to write.
    ImageRecord& v4lImage = g_Images.BeginWrite();
    
    //setup a pointer to the YUYV buffer 
    const unsigned char* yuyv_image = (const unsigned char*)p;

    //a pointer to the destination RGB image
    unsigned char* dest_rgb_image = (unsigned char*)&v4lImage.image[0];

    int iD = 0;
    int iRow = 0;
    int iCol = 0;
    
    for (int i = 0, j = 0; i < width * height * 3; i+=12, j+=8) 
    {
        //first pixel
        y = yuyv_image[j];
        cb = yuyv_image[j+1];
        cr = yuyv_image[j+3];

        r = y + (1.4065 * (cr - 128));
        g = y - (0.3455 * (cb - 128)) - (0.7169 * (cr - 128));
        b = y + (1.7790 * (cb - 128));

        //This prevents colour distortions in your rgb image
        if (r < 0) r = 0;
        else if (r > 255) r = 255;
        if (g < 0) g = 0;
        else if (g > 255) g = 255;
        if (b < 0) b = 0;
        else if (b > 255) b = 255;

        if(iRow < destw && iCol < desth)
        {
            iD = (iCol * destw + iRow) * 3;
            dest_rgb_image[iD] = (unsigned char)r;
            dest_rgb_image[iD+1] = (unsigned char)g;
            dest_rgb_image[iD+2] = (unsigned char)b;
        }

        iRow += 1;       

        if(iRow == width)
        {
            iCol++;
            iRow = 0;
        }

        //second pixel
        y = yuyv_image[j+2];
        cb = yuyv_image[j+1];
        cr = yuyv_image[j+3];

        r = y + (1.4065 * (cr - 128));
        g = y - (0.3455 * (cb - 128)) - (0.7169 * (cr - 128));
        b = y + (1.7790 * (cb - 128));

        if (r < 0) r = 0;
        else if (r > 255) r = 255;
        if (g < 0) g = 0;
        else if (g > 255) g = 255;
        if (b < 0) b = 0;
        else if (b > 255) b = 255;

        if(iRow < destw && iCol < desth)
        {
            iD = (iCol * destw + iRow) * 3;
            dest_rgb_image[iD] = (unsigned char)r;
            dest_rgb_image[iD+1] = (unsigned char)g;
            dest_rgb_image[iD+2] = (unsigned char)b;
        }

        iRow += 1;

        if(iRow == width)
        {
            iCol++;
            iRow = 0;
        }
       
    }

    v4lImage.tick = clock();

    g_Images.FinishWrite();

    g_v4l_profile.OnFrameIter();
}

void* ProcessV4lCamera(void* args)
{
    Config* conf = (Config*)args;

    CaptureSettings cs;

    cs.capture_width = conf->GetInt("v4l_camera_cap_width", 320);
    cs.capture_height = conf->GetInt("v4l_camera_cap_height", 240);
    cs.dest_width = conf->GetInt("col", 160);
    cs.dest_height = conf->GetInt("row", 120);
    const char* devicePath = conf->GetStr("v4l_device_name", "/dev/video0");
    int fps = conf->GetInt("v4l_fps", 60);

    if(!InitV4l(fps, cs.capture_width, cs.capture_height, devicePath))
    {
        printf("failed to init v4l camera.\n");
        return NULL;
    }

    while(programRunning)
    {
        UpdateV4l(process_image, &cs);
    }

    ShutdownV4l();

    return NULL;
}

/////////////////////////////////////////////////////////////////////
//
#include "pointgrey.h"

Profiler g_pg_profile("PGcam", 300);

///Expects a RGB image, resize and then adds to
/// the image queue for others to use.
void process_pg_image(const void *p, int size, void* userData)
{
#ifdef ENABLE_POINT_GREY_CAMERA
    
    CaptureSettings* cs = (CaptureSettings*)userData;

    //we are expecting a RGB frame 640, 480.
    const int width = cs->capture_width;
    const int height = cs->capture_height;
    const int d = 3;

    //we are capturing this portion of the image. In this version
    //we are getting exactly 1/4 of the information, or half
    //native resolution in each dim.
    int destw = cs->dest_width;
    int desth = cs->dest_height;

    //this version assumes an evenly scaled version of w and heigh
    if((width / 4 != destw))
        return;

    if((height / 4 != desth))
        return;

    if(size != (width * height * d))
    {
        printf("wrong image size. Expected: %d, got: %d\n", (width * height * d), size);
        return;
    }

    //get a reference to the next record to write.
    ImageRecord& pgCamImage = g_Images.BeginWrite();
    
    //setup a pointer to the src rgb buffer 
    Pixel* pSrcImage = (Pixel*)p;

    //a pointer to the destination RGB image
    Pixel* pDestImage = (Pixel*)&pgCamImage.image[0];

    int iSx, iSy, iDx, iDy;

    for(iSy = 0, iDy = 0; iSy < height; iSy += 4, iDy++)
        for(iSx = 0, iDx = 0; iSx < width; iSx += 4, iDx++) 
        {
            int iDest = iDx + iDy * destw;
            int iSrc = iSx + iSy * width;

            if(iSrc >= (width * height))
            {
                printf("iSrc out of range: %d, %d\n", iSrc, width * height);
                continue;
            }

            if(iDest >= (destw * desth))
            {
                printf("iDest out of range: %d, %d\n", iDest, destw * desth);                
                continue;
            }

            pDestImage[iDest].r = pSrcImage[iSrc].b;
            pDestImage[iDest].g = pSrcImage[iSrc].g;
            pDestImage[iDest].b = pSrcImage[iSrc].r;
        }

    pgCamImage.tick = clock();

    g_Images.FinishWrite();

    g_pg_profile.OnFrameIter();

#endif //ENABLE_POINT_GREY_CAMERA
}

void* ProcessPGCamera(void* args)
{
#ifdef ENABLE_POINT_GREY_CAMERA
    
    Config* conf = (Config*)args;

    CaptureSettings cs;

    cs.capture_width = conf->GetInt("pg_camera_cap_width", 640);
    cs.capture_height = conf->GetInt("pg_camera_cap_height", 480);
    cs.dest_width = conf->GetInt("col", 160);
    cs.dest_height = conf->GetInt("row", 120);

    printf("Initing point grey camera.\n");

    if(!InitPGCamera(process_pg_image, &cs))
    {
        printf("failed to init point grey camera.\n");
        return NULL;
    }

    while(true)
    {
        //all work done in callback now.
        //UpdatePGCamera(process_pg_image);

        // Restrict rate
        usleep(10000);
    }

    ShutdownPGCamera();

#else

    printf("Point grey camera support disabled.\n");
    
#endif //ENABLE_POINT_GREY_CAMERA

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//Look at the current log dir and see whether we should pickup from a previous recording.

int LastRecordInLogDir(const char* logDir)
{
    int lastRec = 0;

    DIR *dpdf;
    struct dirent *epdf;

    dpdf = opendir(logDir);

    char filename[MAX_PATH_LEN];
    
    if (dpdf != NULL)
    {
        while (epdf = readdir(dpdf))
        {
            strncpy(filename, epdf->d_name, MAX_PATH_LEN);
            
            char* token = strtok(filename, "_");

            if(token != NULL)
            {
                token = strtok(NULL, "_");

                if(token != NULL)
                {
                    int iRec = atoi(token);

                    if(iRec > lastRec)
                        lastRec = iRec;
                }
            }
        }

        lastRec++;

        closedir(dpdf);
    }

    printf("starting recording at: %d\n", lastRec);

    return lastRec;
}

void ensureLogDir(const char* logDir)
{
    struct stat st = {0};

    if (stat(logDir, &st) == -1) {
        mkdir(logDir, 0700);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Save images and axis input pairs. 

void* ProcessLogger(void * args)
{
    Config* conf = (Config*)args;

    //where are we saving log images
    const char* logDir = conf->GetStr("log_dir", "log");

    ensureLogDir(logDir);
    char imagefilename[MAX_PATH_LEN];
    int iRecord = LastRecordInLogDir(logDir);
    bool doRecord = false;
    int width = conf->GetInt("col", 160);
    int height = conf->GetInt("row", 120);
    int num_components = conf->GetInt("ch", 3);
    char num_part[32];
    uint64_t last_buttons = 0;
    uint64_t last_image = 0;
    ButtonRecord button;
    AxisRecord axis;
    ImageRecord* pImage = NULL;
    Profiler profile("Logger", 300);
    int idle_thresh = 1;

    bool debugTestRecording = conf->GetInt("debug_test_recording", 0) == 1;

    if(debugTestRecording)
        doRecord = true;
    
    const int js_button_toggle_logging = conf->GetInt("js_button_toggle_logging", 14);

    //expect at least this much time to pass before we log
    float loggerFpsLimit = 1.0f / conf->GetInt("logger_fps_limit", 60);
    uint64_t lastLog = 0;

    while (programRunning)
    {
        // Restrict rate
        usleep(100);

        g_ButtonInput.Read(button);

        if(button.button != -1 && button.tick != last_buttons)
        {
            if(button.button == js_button_toggle_logging && button.state == 1)
            {
                doRecord = !doRecord;
                led_status(doRecord);
                printf("Record state: %s\n", doRecord ? "True" : "False");

                if(doRecord)
                    ensureLogDir(logDir);
            }

            last_buttons = button.tick;
        }

        
        if(debugTestRecording)
        {
            axis.steer = idle_thresh * 2;
            axis.throttle = idle_thresh * 2;
            axis.tick = s_clock();
            g_AxisInput.Write(axis);
        }

        pImage = g_Images.ReadRef();

        if(doRecord && pImage != NULL
            && g_AxisInput.Read(axis) && pImage->tick != last_image)
        {
            last_image = pImage->tick;

            //only record when we have a non zero throttle. With a small dead zone.
            if(abs(axis.throttle) > idle_thresh)
            {
                blink_led_status(0.5f);

                uint64_t timeToLog = s_clock();

                //only allow logs at a certain rate.
                if(get_sec_diff(timeToLog, lastLog) >= loggerFpsLimit)
                {
                    lastLog = timeToLog;

                    //write image and steering pair to the log.
                    sprintf(num_part, "%08d", iRecord);
                    sprintf(imagefilename, "%s/img_%s_st_%d_th_%d.jpg", logDir, num_part, axis.steer, axis.throttle);

                    if ( !tje_encode_to_file(imagefilename, width, height, num_components, (const unsigned char*)pImage->image) ) 
                    {
                        fprintf(stderr, "Could not write JPEG\n");
                    }                    

                    iRecord++;
                }
            }
            else
            {
                blink_led_status(1.0f);
            }

            profile.OnFrameIter();  
        }
        
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Save images and axis input pairs. 

void* ProcessRobot(void * args)
{
#if ENABLE_CAR

    Config* conf = (Config*)args;

    Car car;
    PwmServoConfig steeringConfig;
    PwmEscConfig escConfig;

    steeringConfig.channel      = conf->GetInt("ada_steering_servo_channel", 1);
	steeringConfig.frequency 	= conf->GetInt("pwm_servo_freq", 60);
	steeringConfig.resolution 	= conf->GetInt("pwm_servo_resolution", 4096);
	steeringConfig.posInit 	    = conf->GetInt("steering_mid", 400);
	steeringConfig.posStraight  = conf->GetInt("steering_mid", 400);
	steeringConfig.posMinLeft 	= conf->GetInt("steering_low", 310);
	steeringConfig.posMaxLeft 	= conf->GetInt("steering_low", 310);
	steeringConfig.posMinRight  = conf->GetInt("steering_hi", 530);
	steeringConfig.posMaxRight  = conf->GetInt("steering_hi", 530);


    escConfig.channel       = conf->GetInt("ada_esc_motor_channel", 2);
    escConfig.frequency 	= conf->GetInt("pwm_servo_freq", 60);
	escConfig.resolution 	= conf->GetInt("pwm_servo_resolution", 4096);
	escConfig.posInit 	    = conf->GetInt("esc_init_mid", 400);
	escConfig.posIdle       = conf->GetInt("esc_throttle_mid", 400);
	escConfig.posMinForward	= conf->GetInt("esc_throttle_hi", 450);
	escConfig.posMaxForward	= conf->GetInt("esc_init_hi", 600);
	escConfig.posMinReverse	= conf->GetInt("esc_throttle_lo", 350);
	escConfig.posMaxReverse	= conf->GetInt("esc_init_lo", 200);

    car.init(&steeringConfig, &escConfig);

    //local copy of axis and pred records
    AxisRecord axis, pred;

    //timestamp of last pred
    uint64_t lastPred = 0;
    
    //scale inputs from joystick on this axis range
    float axisRange = conf->GetFloat("js_axis_scale", 32767.0f);

    //while this is non zero, we are letting the prediction steer
    int predSteer = 0;
    int predThrottle = 0;

    Profiler profile("Robot", 300);

	car.printStatus();

	if (car.isReady()) 
    {
        while(programRunning)
        {
             // Restrict rate
            usleep(10000);

            if(g_PredInput.Read(pred) && lastPred != pred.tick)
            {
                lastPred = pred.tick;

                float steering = (float)pred.steer / axisRange;
                car.setSteering(steering);

                predSteer = 60;

                float throttle = (float)pred.throttle / axisRange;
                //printf("pred_throttle: %f\n", throttle);
                car.setThrottle(throttle);

                //zero throttle means user can interact
                if( throttle != 0.0f)
                    predThrottle = 60;
            }

            if(predSteer > 0)
                predSteer--;
            if(predThrottle > 0)
                predThrottle--;

            if(g_AxisInput.Read(axis))
            {
                float throttle = (float)axis.throttle / axisRange;
                float steering = (float)axis.steer / axisRange;

                //allow prediction to win when steering.
                if(predSteer == 0)
                    car.setSteering(steering);

                //we always control throttle for now.
                if(predThrottle == 0)
                {  
                    car.setThrottle(throttle);
                }
            }
            
            profile.OnFrameIter();
        }
    }

    #endif //#if ENABLE_CAR

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// Adapter to push images to our networked NN and retrieve steering and 
// throttle predictions

void* ProcessKerasPredictions(void * args)
{
    Config* conf = (Config*)args;
    AxisRecord axis;
    ButtonRecord button;
    ImageRecord image;
    uint64_t last_button = 0;
    uint64_t last_image = 0;

    int img_port = conf->GetInt("keras_predict_server_img_port", 9090);
    void *context = zmq_ctx_new ();
    void *socket = zmq_socket (context, ZMQ_REQ);
    char connection[MAX_STR_LEN];
    sprintf(connection, "tcp://127.0.0.1:%d", img_port);
    printf("looking for pred server at %s\n", connection);
    zmq_connect(socket, connection);

    Profiler profile("Prediction", 300);
    Json j(32); //max 32 tokens.
    char buffer [1024];
    bool doPredict = false;

    bool debugTestPredict = conf->GetInt("debug_test_predict", 0) == 1;

    if(debugTestPredict)
        doPredict = true;
    
    //Init image dimensions
    const int rows = conf->GetInt("row", 120);
    const int cols = conf->GetInt("col", 160);
    const int ch = conf->GetInt("ch", 3);
    const int max_image_len = rows * cols * ch;

    const int js_button_toggle_sd = conf->GetInt("js_button_toggle_sd", 12);
    const int js_button_dpad_up = conf->GetInt("js_button_dpad_up", 4);
    const int js_button_dpad_down = conf->GetInt("js_button_dpad_down", 6);

    float speed_scalar = 1.0f;
    float axisRange = conf->GetFloat("js_axis_scale", 32767.0f);

    while(programRunning)
    {
        // Restrict rate
        usleep(1000);

        if(g_ButtonInput.Read(button) && button.tick != last_button)
        {
            //keep track of last button read
            last_button = button.tick;

            //12=Triangle on the PS3 sixaxis controller
            if(button.button == js_button_toggle_sd && button.state == 1)
            {
                doPredict = !doPredict;
                
                led_status(doPredict);

                printf("Prediction: %s\n", doPredict ? "on" : "off");

                if(doPredict)
                    profile.Reset();
            }
            
            //dpad up
            if(button.button == js_button_dpad_up && button.state == 1)
            {
                speed_scalar += 0.05f;
                printf("scale speed: %f\n", speed_scalar);
            }

            //dpad down
            if(button.button == js_button_dpad_down && button.state == 1)
            {
                speed_scalar -= 0.2f;
                printf("scale speed: %f\n", speed_scalar);
            }
        }

        if(doPredict && g_Images.Read(image) && image.tick != last_image)
        {
            //keep track of last image read
            last_image = image.tick;

            //send image to predictor
            send_message(socket, image.image, max_image_len);

            //receive steering and throttle. This will block.
            int count = zmq_recv (socket, buffer, 1024, 0);

            //null terminate
            buffer[count] = 0;

            if (j.Parse(buffer))
            {
                axis.steer = j.GetElemFloat("steering", 0);
                axis.throttle = j.GetElemFloat("throttle", 0);

                //scale speed
                axis.throttle *= speed_scalar;
                
                //blink when we are active.
                blink_led_status(0.5f);
                
                axis.tick = clock();

                //post prediction to our ring buffer
                g_PredInput.Write(axis);
            }

            profile.OnFrameIter();
           
        }
    }

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// Reply to webserver with updates from our camera

void* ProcessWebUpdate(void * args)
{
    Config* conf = (Config*)args;  
    ImageRecord image;
    uint64_t last_image = 0;
    int web_img_port = conf->GetInt("web_image_port", 9191);
    void *context = zmq_ctx_new ();
    void *socket = zmq_socket (context, ZMQ_REP);
    char connection[MAX_STR_LEN];
    sprintf(connection, "tcp://*:%d", web_img_port);
    printf("listening for web image requests on: %s\n", connection);
    zmq_bind(socket, connection);
    char buffer [1024];

    //Init image dimensions
    const int rows = conf->GetInt("row", 120);
    const int cols = conf->GetInt("col", 160);
    const int ch = conf->GetInt("ch", 3);
    const int max_image_len = rows * cols * ch;


    while(programRunning)
    {
        //this just blocks until it gets a request.
        int count = zmq_recv (socket, buffer, 1024, 0);

        //wait for a new image. not likely very long, unless
        //camera is down.
        while(!g_Images.Read(image))
        {
            usleep(10000);
        }

        //keep track of last image read
        last_image = image.tick;

        //send image to web server
        send_message(socket, image.image, max_image_len);
    }
}

// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void
signal_callback_handler(int signum)
{
    printf("Caught signal %d\n",signum);

    //signal threads to stop
    programRunning = false;

    //wait for them to stop
    usleep(1000000);

    //ok we are done, hopefully
    printf("program terminating.\n");

    // Terminate program
    exit(signum);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// main loop

int main(int argc, char** argv)
{
    programRunning = true;

    // Register signal and signal handler
    signal(SIGINT, signal_callback_handler);

    fprintf(stdout,"%s Version %d.%d\n",
            argv[0],
            shark_VERSION_MAJOR,
            shark_VERSION_MINOR);

    Config conf;
    CameraType activeCameraType = Cam_V4l;
    PreditionType predType = Pred_Keras;

    std::string config_filename = "config.json";

    ////////////////////////////////
    // Parse command line options

    for(int iArg = 1; iArg < argc; iArg++)
    {
        char* arg = argv[iArg];

        if(0 == strcmp(arg, "--config") && (iArg + 1) < argc)
        {
            config_filename = argv[iArg + 1];
            iArg++;
        }
    }


    /////////////////////////////////
    // Load and parse json configuration file

    if(!conf.Load(config_filename.c_str()))
        return -1;
    else
    {
        printf("Loaded config: %s\n", config_filename.c_str());
    }

    
    /////////////////////////////////
    //select camera type
    const char* camera_type = conf.GetStr("camera_type", "V4L");

    if(strcmp(camera_type, "Raspicam") == 0)
        activeCameraType = Cam_Raspi;
    else if(strcmp(camera_type, "V4L") == 0)
        activeCameraType = Cam_V4l;
    else if(strcmp(camera_type, "PointGrey") == 0)
        activeCameraType = Cam_PointGrey;


    /////////////////////////
    // Init image records
    InitImageRecordSize(conf);

    /////////////////////////////////
    // Launch our worker threads

    pthread_t thread_js;
    pthread_create(&thread_js, NULL, ProcessJoyStick, &conf);

    pthread_t thread_cam;
    if(activeCameraType == Cam_V4l)
        pthread_create(&thread_cam, NULL, ProcessV4lCamera, &conf);
    else if(activeCameraType == Cam_PointGrey)
        pthread_create(&thread_cam, NULL, ProcessPGCamera, &conf);
    else if(activeCameraType == Cam_Raspi)
        pthread_create(&thread_cam, NULL, ProcessRaspiCamera, &conf);

    pthread_t thread_log;
    pthread_create(&thread_log, NULL, ProcessLogger, &conf);

    pthread_t thread_robot;
    pthread_create(&thread_robot, NULL, ProcessRobot, &conf);

    pthread_t thread_pred;
    if(predType == Pred_Keras)
        pthread_create(&thread_pred, NULL, ProcessKerasPredictions, &conf);
    
    pthread_t thread_web_update;
    pthread_create(&thread_web_update, NULL, ProcessWebUpdate, &conf);

    /////////////////////////////////
    // wait for worker threads to exit

    pthread_join(thread_js, NULL);
    printf("js thread exited.\n");
    pthread_join(thread_cam, NULL);
    printf("cam thread exited.\n");
    pthread_join(thread_log, NULL);
    printf("log thread exited.\n");
    pthread_join(thread_robot, NULL);
    printf("robot thread exited.\n");
    pthread_join(thread_pred, NULL);
    printf("pred thread exited.\n");
    pthread_join(thread_web_update, NULL);
    printf("web update thread exited.\n");

    programExited = true;

    return 0;
}
