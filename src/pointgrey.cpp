#include "pointgrey.h"

#ifdef ENABLE_POINT_GREY_CAMERA

#include <stdio.h>
#include "C/FlyCapture2_C.h"


void PrintBuildInfo()
{
    fc2Version version;
    char versionStr[512];
    char timeStamp[512];

    fc2GetLibraryVersion(&version);

    sprintf(versionStr,
            "FlyCapture2 library version: %d.%d.%d.%d\n",
            version.major,
            version.minor,
            version.type,
            version.build);

    printf("%s", versionStr);

    sprintf(timeStamp, "Application build date: %s %s\n\n", __DATE__, __TIME__);

    printf("%s", timeStamp);
}

void PrintCameraInfo(fc2Context context)
{
    fc2Error error;
    fc2CameraInfo camInfo;
    error = fc2GetCameraInfo(context, &camInfo);
    if (error != FC2_ERROR_OK)
    {
        // Error
    }

    printf("\n*** CAMERA INFORMATION ***\n"
           "Serial number - %u\n"
           "Camera model - %s\n"
           "Camera vendor - %s\n"
           "Sensor - %s\n"
           "Resolution - %s\n"
           "Firmware version - %s\n"
           "Firmware build time - %s\n\n",
           camInfo.serialNumber,
           camInfo.modelName,
           camInfo.vendorName,
           camInfo.sensorInfo,
           camInfo.sensorResolution,
           camInfo.firmwareVersion,
           camInfo.firmwareBuildTime);
}

int print_error(fc2Error error)
{
     if (error == FC2_ERROR_UNDEFINED) printf("FC2_ERROR_UNDEFINED\n");
     else if (error == FC2_ERROR_OK) printf("FC2_ERROR_OK\n");
     else if (error == FC2_ERROR_FAILED) printf("FC2_ERROR_FAILED\n");
     else if (error == FC2_ERROR_NOT_IMPLEMENTED) printf("FC2_ERROR_NOT_IMPLEMENTED\n");
     else if (error == FC2_ERROR_FAILED_BUS_MASTER_CONNECTION) printf("FC2_ERROR_FAILED_BUS_master_connection\n");
     else if (error == FC2_ERROR_NOT_CONNECTED) printf("FC2_ERROR_NOT_CONNECTED\n");
     else if (error == FC2_ERROR_INIT_FAILED) printf("FC2_ERROR_INIT_FAILED\n");
     else if (error == FC2_ERROR_NOT_INTITIALIZED) printf("FC2_ERROR_NOT_INTITIALIZED\n");
     else if (error == FC2_ERROR_INVALID_PARAMETER) printf("FC2_ERROR_INVALID_PARAMETER\n");
     else if (error == FC2_ERROR_INVALID_SETTINGS) printf("FC2_ERROR_INVALID_SETTINGS\n");
     else if (error == FC2_ERROR_INVALID_BUS_MANAGER) printf("FC2_ERROR_INVALID_BUS_manager\n");
     else if (error == FC2_ERROR_MEMORY_ALLOCATION_FAILED) printf("FC2_ERROR_MEMORY_ALLOCATION_failed\n");
     else if (error == FC2_ERROR_LOW_LEVEL_FAILURE) printf("FC2_ERROR_LOW_LEVEL_failure\n");
     else if (error == FC2_ERROR_NOT_FOUND) printf("FC2_ERROR_NOT_FOUND\n");
     else if (error == FC2_ERROR_FAILED_GUID) printf("FC2_ERROR_FAILED_GUID\n");
     else if (error == FC2_ERROR_INVALID_PACKET_SIZE) printf("FC2_ERROR_INVALID_PACKET_size\n");
     else if (error == FC2_ERROR_INVALID_MODE) printf("FC2_ERROR_INVALID_MODE\n");
     else if (error == FC2_ERROR_NOT_IN_FORMAT7) printf("FC2_ERROR_NOT_IN_format7\n");
     else if (error == FC2_ERROR_NOT_SUPPORTED) printf("FC2_ERROR_NOT_SUPPORTED\n");
     else if (error == FC2_ERROR_TIMEOUT) printf("FC2_ERROR_TIMEOUT\n");
     else if (error == FC2_ERROR_BUS_MASTER_FAILED) printf("FC2_ERROR_BUS_MASTER_failed\n");
     else if (error == FC2_ERROR_INVALID_GENERATION) printf("FC2_ERROR_INVALID_GENERATION\n");
     else if (error == FC2_ERROR_LUT_FAILED) printf("FC2_ERROR_LUT_FAILED\n");
     else if (error == FC2_ERROR_IIDC_FAILED) printf("FC2_ERROR_IIDC_FAILED\n");
     else if (error == FC2_ERROR_STROBE_FAILED) printf("FC2_ERROR_STROBE_FAILED\n");
     else if (error == FC2_ERROR_TRIGGER_FAILED) printf("FC2_ERROR_TRIGGER_FAILED\n");
     else if (error == FC2_ERROR_PROPERTY_FAILED) printf("FC2_ERROR_PROPERTY_FAILED\n");
     else if (error == FC2_ERROR_PROPERTY_NOT_PRESENT) printf("FC2_ERROR_PROPERTY_NOT_present\n");
     else if (error == FC2_ERROR_REGISTER_FAILED) printf("FC2_ERROR_REGISTER_FAILED\n");
     else if (error == FC2_ERROR_READ_REGISTER_FAILED) printf("FC2_ERROR_READ_REGISTER_failed\n");
     else if (error == FC2_ERROR_WRITE_REGISTER_FAILED) printf("FC2_ERROR_WRITE_REGISTER_failed\n");
     else if (error == FC2_ERROR_ISOCH_FAILED) printf("FC2_ERROR_ISOCH_FAILED\n");
     else if (error == FC2_ERROR_ISOCH_ALREADY_STARTED) printf("FC2_ERROR_ISOCH_ALREADY_started\n");
     else if (error == FC2_ERROR_ISOCH_NOT_STARTED) printf("FC2_ERROR_ISOCH_NOT_started\n");
     else if (error == FC2_ERROR_ISOCH_START_FAILED) printf("FC2_ERROR_ISOCH_START_failed\n");
     else if (error == FC2_ERROR_ISOCH_RETRIEVE_BUFFER_FAILED) printf("FC2_ERROR_ISOCH_RETRIEVE_buffer_failed\n");
     else if (error == FC2_ERROR_ISOCH_STOP_FAILED) printf("FC2_ERROR_ISOCH_STOP_failed\n");
     else if (error == FC2_ERROR_ISOCH_SYNC_FAILED) printf("FC2_ERROR_ISOCH_SYNC_failed\n");
     else if (error == FC2_ERROR_ISOCH_BANDWIDTH_EXCEEDED) printf("FC2_ERROR_ISOCH_BANDWIDTH_exceeded\n");
     else if (error == FC2_ERROR_IMAGE_CONVERSION_FAILED) printf("FC2_ERROR_IMAGE_CONVERSION_failed\n");
     else if (error == FC2_ERROR_IMAGE_LIBRARY_FAILURE) printf("FC2_ERROR_IMAGE_LIBRARY_failure\n");
     else if (error == FC2_ERROR_BUFFER_TOO_SMALL) printf("FC2_ERROR_BUFFER_TOO_small\n");
     else if (error == FC2_ERROR_IMAGE_CONSISTENCY_ERROR) printf("FC2_ERROR_IMAGE_CONSISTENCY_error\n");
     else if (error == FC2_ERROR_FORCE_32BITS) printf("FC2_ERROR_FORCE_32BITS\n");
     else printf("FC2_ERROR_UNknown %d\n",error);

     return 0;
}

void SetTimeStamping(fc2Context context, BOOL enableTimeStamp)
{
    fc2Error error;
    fc2EmbeddedImageInfo embeddedInfo;

    error = fc2GetEmbeddedImageInfo(context, &embeddedInfo);
    if (error != FC2_ERROR_OK)
    {
        printf("Error in fc2GetEmbeddedImageInfo: %d\n", error);
    }

    if (embeddedInfo.timestamp.available != 0)
    {
        embeddedInfo.timestamp.onOff = enableTimeStamp;
    }

    error = fc2SetEmbeddedImageInfo(context, &embeddedInfo);
    if (error != FC2_ERROR_OK)
    {
        printf("Error in fc2SetEmbeddedImageInfo: %d\n", error);
    }
}

struct PGCamera
{
    fc2Error error;
    fc2Context context;
    fc2PGRGuid guid;
    fc2Image rawImage;
    fc2Image convertedImage;
    fc2Config config;
    process_pg_image_cb cb;
};


PGCamera g_pgCamera;

void ImageEventCallback( fc2Image* image, void* pCallbackData );

bool InitPGCamera(process_pg_image_cb cb, void* userData)
{
    fc2Error error;
    unsigned int numCameras = 0;
    g_pgCamera.cb = cb;

    PrintBuildInfo();

    error = fc2CreateContext(&g_pgCamera.context);
    if (error != FC2_ERROR_OK)
    {
        printf("Error in fc2CreateContext: %d\n", error);
        return false;
    }

    error = fc2GetNumOfCameras(g_pgCamera.context, &numCameras);
    if (error != FC2_ERROR_OK)
    {
        fc2DestroyContext(g_pgCamera.context);

        printf("Error in fc2GetNumOfCameras: %d\n", error);
        print_error(error);
        return false;
    }

    if (numCameras == 0)
    {
        fc2DestroyContext(g_pgCamera.context);

        // No cameras detected
        printf("No cameras detected.\n");
        return false;
    }

    // Get the 0th camera
    error = fc2GetCameraFromIndex(g_pgCamera.context, 0, &g_pgCamera.guid);
    if (error != FC2_ERROR_OK)
    {
        fc2DestroyContext(g_pgCamera.context);

        printf("Error in fc2GetCameraFromIndex: %d\n", error);
        return false;
    }

    error = fc2Connect(g_pgCamera.context, &g_pgCamera.guid);
    if (error != FC2_ERROR_OK)
    {
        fc2DestroyContext(g_pgCamera.context);

        printf("Error in fc2Connect: %d\n", error);
        return false;
    }

    PrintCameraInfo(g_pgCamera.context);

    
    error = fc2CreateImage(&g_pgCamera.rawImage);
    if (error != FC2_ERROR_OK)
    {
        printf("Error in fc2CreateImage: %d\n", error);
        return false;
    }

    error = fc2CreateImage(&g_pgCamera.convertedImage);
    if (error != FC2_ERROR_OK)
    {
        printf("Error in fc2CreateImage: %d\n", error);
        return false;
    }
    
    BOOL    supported = 0;
    fc2VideoMode videoMode = FC2_VIDEOMODE_640x480Y8; 
	fc2FrameRate frameRate = FC2_FRAMERATE_60; 
				
    error = fc2GetVideoModeAndFrameRateInfo(
				g_pgCamera.context,
				videoMode,
				frameRate,
				&supported);

    if(supported)
    {
        error = fc2SetVideoModeAndFrameRate(
				g_pgCamera.context,
				videoMode,
				frameRate);
	
        if (error != FC2_ERROR_OK)
        {
            printf("Error in fc2SetVideoModeAndFrameRate: %d\n", error);
            return false;
        }
        else
        {
            printf("Video mode supported. setup now.\n");
            
        }
    }
    else
    {
        printf("Video mode not supported. using defaults.\n");
    }


    error = fc2StartCaptureCallback(g_pgCamera.context, ImageEventCallback, userData);
    if (error != FC2_ERROR_OK)
    {
        fc2DestroyContext(g_pgCamera.context);

        printf("Error in fc2SetCallback: %d\n", error);
        return false;
    }

    return true;
}

void ImageEventCallback( fc2Image* image, void* pCallbackData )
{
    fc2Error error;

    error = fc2ConvertImageTo(FC2_PIXEL_FORMAT_BGR, image, &g_pgCamera.convertedImage);
    
    if (error != FC2_ERROR_OK)
    {
        printf("Error in fc2ConvertImageTo: %d\n", error);
        print_error(error);
        return;
    }

    void* pData = g_pgCamera.convertedImage.pData;
    int sizeData = g_pgCamera.convertedImage.dataSize;
    g_pgCamera.cb(pData, sizeData, pCallbackData);
    
    //g_pgCamera.nextReady = true;
}

void UpdatePGCamera(process_pg_image_cb cb)
{
    //all work done in callback. This method resulted in tearing.

    /*
    // Retrieve the image
    error = fc2RetrieveBuffer(g_pgCamera.context, &g_pgCamera.rawImage);
    if (error != FC2_ERROR_OK)
    {
        printf("Error in retrieveBuffer: %d\n", error);
        print_error(error);
        return;
    }

    // Convert the final image to RGB
    error = fc2ConvertImageTo(FC2_PIXEL_FORMAT_BGR, &g_pgCamera.rawImage, &g_pgCamera.convertedImage);
    if (error != FC2_ERROR_OK)
    {
        printf("Error in fc2ConvertImageTo: %d\n", error);
        print_error(error);
        return;
    }
    */  
}

void ShutdownPGCamera()
{
    fc2Error error;

    error = fc2DestroyImage(&g_pgCamera.rawImage);
    if (error != FC2_ERROR_OK)
    {
        printf("Error in fc2DestroyImage: %d\n", error);
    }

    error = fc2DestroyImage(&g_pgCamera.convertedImage);
    if (error != FC2_ERROR_OK)
    {
        printf("Error in fc2DestroyImage: %d\n", error);
    }


    error = fc2StopCapture(g_pgCamera.context);
    if (error != FC2_ERROR_OK)
    {
        fc2DestroyContext(g_pgCamera.context);

        printf("Error in fc2StopCapture: %d\n", error);
    }

    error = fc2DestroyContext(g_pgCamera.context);
    if (error != FC2_ERROR_OK)
    {
        printf("Error in fc2DestroyContext: %d\n", error);
    }
}

#endif //ENABLE_POINT_GREY_CAMERA
