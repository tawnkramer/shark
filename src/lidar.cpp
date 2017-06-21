#include <stdlib.h>
#include "lidar.h"

#if ENABLE_RPLIDAR

#include "rplidar/rplidar.h" //RPLIDAR standard sdk, all-in-one header

#ifndef _countof
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))
#endif

using namespace rp::standalone::rplidar;

// the driver instance
RPlidarDriver * g_pDrv = NULL;
bool bVerboseLidarOutput = false;

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


bool InitLidar(Config* conf) 
{
    const char * opt_com_path = conf->GetStr("lidar_dev_file", "/dev/ttyUSB0");
    _u32         opt_com_baudrate = 115200;
    u_result     op_result;

    bVerboseLidarOutput = conf->GetInt("lidar_verbose_output", 0);

    // create the driver instance
    g_pDrv = RPlidarDriver::CreateDriver(RPlidarDriver::DRIVER_TYPE_SERIALPORT);
    
    if (!g_pDrv) 
    {
        fprintf(stderr, "insufficent memory, exit\n");
        return false;
    }

    bool problems = false;

    // make connection...
    if (IS_FAIL(g_pDrv->connect(opt_com_path, opt_com_baudrate))) {
        fprintf(stderr, "Error, cannot bind to the specified serial port %s.\n"
            , opt_com_path);
        problems = true;
    }

    rplidar_response_device_info_t devinfo;

	// retrieving the device info
    ////////////////////////////////////////
    if(!problems)
    {
        op_result = g_pDrv->getDeviceInfo(devinfo);

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
        problems = !checkRPLIDARHealth(g_pDrv);
    }
	
    if(!problems)
    {
        g_pDrv->startMotor();
        // start scan...
        g_pDrv->startScan();
    }
    else
    {
        ShutdownLidar();
    }

    return !problems;
}

void UpdateLidar()
{
   if(g_pDrv == NULL)
        return;

    rplidar_response_measurement_node_t nodes[360*2];
    size_t   count = _countof(nodes);

    u_result op_result = g_pDrv->grabScanData(nodes, count);

    if (IS_OK(op_result)) 
    {
        g_pDrv->ascendScanData(nodes, count);
    
        if(bVerboseLidarOutput)
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
    }
}

void ShutdownLidar()
{
    if(g_pDrv == NULL)
        return;

    g_pDrv->stop();
    g_pDrv->stopMotor();
    RPlidarDriver::DisposeDriver(g_pDrv);
    g_pDrv = NULL;
}

#else //ENABLE_RPLIDAR

bool InitLidar(Config* conf) { return false; }
void UpdateLidar(){}
void ShutdownLidar(){}

#endif //ENABLE_RPLIDAR
