#ifndef __LIDAR_H__
#define __LIDAR_H__

#include "SharkConfig.h"
#include "config.h"

//Modelled after rplidar return
struct LidarRet
{
    unsigned char quality;
    unsigned short angle;
    unsigned short distance;

    float GetDistance() { return distance / 4.0f; }
    float GetAngle() { return (angle >> 1) / 64.0f; }
    int GetQuality() { return (quality >> 2); }
};

struct LidarRetSet
{
    enum Constants
    {
        NUM_LIDAR_RETURNS = 360 * 2,
    };

    LidarRetSet() : count(NUM_LIDAR_RETURNS) 
    {

    }

    int count;
    LidarRet m_Returns[NUM_LIDAR_RETURNS];
};

//user callback to process a frame.
typedef void (*process_lidar_cb)(const LidarRetSet *p, void* userData);

bool InitLidar(Config* pConfig, void* userData = NULL);
void UpdateLidar(process_lidar_cb cb);
void ShutdownLidar();


#endif //__LIDAR_H__
