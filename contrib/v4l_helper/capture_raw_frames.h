#ifndef __CAP_RAW_FRAMES_H__
#define __CAP_RAW_FRAMES_H__

#include <stdbool.h>

#define   FPS_187 187
#define   FPS_150 150
#define   FPS_137 137
#define   FPS_125 125
#define   FPS_100 100
#define   FPS_60 60
#define   FPS_50 50
#define   FPS_37 37
#define   FPS_30 30


//C linkage in C++
#ifdef __cplusplus
extern "C" {
#endif


//user callback to process a frame.
typedef void (*process_image_cb)(const void *p, int size, void* userData);

//Init video 4 linux device. returns false on failure.
bool InitV4l(int fps, int width, int height, const char* devicePath);

//Poll the video device and callback with any image data
void UpdateV4l(process_image_cb cb, void* userData);

//Shutdown video devices
void ShutdownV4l();


//C linkage in C++
#ifdef __cplusplus
}
#endif

#endif //__CAP_RAW_FRAMES_H__

