#ifndef __LIDAR_H__
#define __LIDAR_H__

#include "SharkConfig.h"
#include "config.h"

bool InitLidar(Config* pConfig);
void UpdateLidar();
void ShutdownLidar();


#endif //__LIDAR_H__
