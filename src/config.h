#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "json.h"
#include <string>

#define MAX_SETTINGS 256

class Config
{
public:

    Config() : settings(MAX_SETTINGS) {}

    bool        Load(const char* filename);

    const char* GetStr(const char* setting, const char* defaultVal=NULL);
    int         GetInt(const char* setting, int unfoundVal=-1);
    float       GetFloat(const char* setting, float unfoundVal=0.0f);

protected:

    Json settings;
    std::string strData;
};

#endif //__CONFIG_H__
