#include <stdlib.h>
#include <string.h>
#include "lidar.h"

#if ENABLE_RPLIDAR

#include "rplidar/rplidar.h" //RPLIDAR standard sdk, all-in-one header

#ifndef _countof
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))
#endif

using namespace rp::standalone::rplidar;

// the driver instance
struct RPLidarMan
{
    RPLidarMan() {
        m_pDrv = NULL;
        m_bVerboseLidarOutput = false;
        m_pUserData = NULL;
    }

    RPlidarDriver * m_pDrv;
    bool m_bVerboseLidarOutput;
    void* m_pUserData;
    LidarRetSet m_Set;
};

RPLidarMan g_Lidar;

bool checkRPLIDARHealth(RPlidarDriver * drv)
{
    u_result     op_result;
    rplidar_response_device_health_t healthinfo;


    op_result = drv->getHealth(healthinfo);
    if (IS_OK(op_result)) { // the macro IS_OK is the preperred way to judge whether the operation is succeed.
        printf("RPLidar health status : %d\n", healthinfo.status);
        if (healthinfo.status == RPLIDAR_STATUS_ERROR) {
            fprintf(stderr, "Error, rplidar internal error detected. Please reboot the device to retry.\n");
            // enable the following code if you want rplidar to be reboot by software
            // drv->reset();
            return false;
        } else {
            return true;
        }

    } else {
        fprintf(stderr, "Error, cannot retrieve the lidar health code: %x\n", op_result);
        return false;
    }
}


bool InitLidar(Config* conf, void* userData) 
{
    g_Lidar.m_pUserData = userData;

    const char * opt_com_path = conf->GetStr("lidar_dev_file", "/dev/ttyUSB0");
    _u32         opt_com_baudrate = 115200;
    u_result     op_result;

    g_Lidar.m_bVerboseLidarOutput = conf->GetInt("lidar_verbose_output", 0);

    // create the driver instance
    g_Lidar.m_pDrv = RPlidarDriver::CreateDriver(RPlidarDriver::DRIVER_TYPE_SERIALPORT);
    
    if (!g_Lidar.m_pDrv) 
    {
        fprintf(stderr, "insufficent memory, exit\n");
        return false;
    }

    bool problems = false;

    // make connection...
    if (IS_FAIL(g_Lidar.m_pDrv->connect(opt_com_path, opt_com_baudrate))) {
        fprintf(stderr, "Error, cannot bind to the specified serial port %s.\n"
            , opt_com_path);
        problems = true;
    }

    rplidar_response_device_info_t devinfo;

	// retrieving the device info
    ////////////////////////////////////////
    if(!problems)
    {
        op_result = g_Lidar.m_pDrv->getDeviceInfo(devinfo);

        if (IS_FAIL(op_result)) {
            fprintf(stderr, "Error, cannot get device info.\n");
            problems = true;
        }
    }

    if(!problems)
    {
        // print out the device serial number, firmware and hardware version number..
        printf("RPLIDAR S/N: ");
        for (int pos = 0; pos < 16 ;++pos) {
            printf("%02X", devinfo.serialnum[pos]);
        }

        printf("\n"
                "Firmware Ver: %d.%02d\n"
                "Hardware Rev: %d\n"
                , devinfo.firmware_version>>8
                , devinfo.firmware_version & 0xFF
                , (int)devinfo.hardware_version);
    }

    // check health...
    if (!problems)
    {
        problems = !checkRPLIDARHealth(g_Lidar.m_pDrv);
    }
	
    if(!problems)
    {
        g_Lidar.m_pDrv->startMotor();
        // start scan...
        g_Lidar.m_pDrv->startScan();
    }
    else
    {
        ShutdownLidar();
    }

    return !problems;
}

void UpdateLidar(process_lidar_cb cb)
{
   if(g_Lidar.m_pDrv == NULL)
        return;

    rplidar_response_measurement_node_t nodes[360*2];
    size_t   count = _countof(nodes);

    memset(nodes, 0, sizeof(nodes));
    
    u_result op_result = g_Lidar.m_pDrv->grabScanData(nodes, count);

    if (IS_OK(op_result)) 
    {
        g_Lidar.m_pDrv->ascendScanData(nodes, count);
    
        if(g_Lidar.m_bVerboseLidarOutput)
        {
            for (int pos = 0; pos < (int)count ; ++pos) 
            {
                printf("%s theta: %03.2f Dist: %08.2f Q: %d \n", 
                    (nodes[pos].sync_quality & RPLIDAR_RESP_MEASUREMENT_SYNCBIT) ?"S ":"  ", 
                    (nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT)/64.0f,
                    nodes[pos].distance_q2/4.0f,
                    nodes[pos].sync_quality >> RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT);
            }
        }

        //reurn size doesn't quite match my structure size yet. TODO.. probably because of the __atribute__ packed. figure
        //this out and do a memcpy. Probably much faster. Though on the jetson, this doesn't add any appreciable delay.
        if(sizeof(nodes) == sizeof(g_Lidar.m_Set.m_Returns))
        {
            memcpy(&g_Lidar.m_Set.m_Returns[0], &nodes[0], sizeof(nodes));
        }
        else
        {
            //do a slower element by element copy
            g_Lidar.m_Set.m_Count = count;

            for (int pos = 0; pos < (int)count ; ++pos) 
            {
                LidarRet& ret = g_Lidar.m_Set.m_Returns[pos];
                rplidar_response_measurement_node_t& n = nodes[pos];
                ret.quality = n.sync_quality;
                ret.angle = n.angle_q6_checkbit;
                ret.distance = n.distance_q2;
            }
        }

        cb(&g_Lidar.m_Set, g_Lidar.m_pUserData);
    }
}

void ShutdownLidar()
{
    if(g_Lidar.m_pDrv == NULL)
        return;

    g_Lidar.m_pDrv->stop();
    g_Lidar.m_pDrv->stopMotor();
    RPlidarDriver::DisposeDriver(g_Lidar.m_pDrv);
    g_Lidar.m_pDrv = NULL;
}

#else //ENABLE_RPLIDAR

bool InitLidar(Config* conf, void* userdata) { return false; }
void UpdateLidar(process_lidar_cb cb){}
void ShutdownLidar(){}

#endif //ENABLE_RPLIDAR
