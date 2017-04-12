#ifndef ___POINT_GREY_H__
#define ___POINT_GREY_H__

#include "SharkConfig.h"

//should be enabled automagically if the cmake detects you have the sdk installed
#if ENABLE_PG_SDK
    #define ENABLE_POINT_GREY_CAMERA
#endif

//user callback to process a frame.
typedef void (*process_pg_image_cb)(const void *p, int size, void* userData);

bool InitPGCamera(process_pg_image_cb, void* userData);
void UpdatePGCamera(process_pg_image_cb);
void ShutdownPGCamera();

#endif //___POINT_GREY_H__
