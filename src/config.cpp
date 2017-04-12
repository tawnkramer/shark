#include <fstream>
#include "config.h"

///////////////////////////////////////////////////////////////////////////////
// read a json file for config settings
// normally a json parser doesn't handle comments
// so we do some preprocessing to remove comments with //

bool Config::Load(const char* filename)
{
    std::ifstream file(filename);
    std::string str;
    strData.clear();
    std::string comment("//");

    while (std::getline(file, str))
    {
        //skip comment line
        if(str.compare(0, comment.length(), comment) == 0)
            continue;

        //strip out any comment contents at the end of a line.
        std::size_t pos = str.find(comment);

        if(pos != std::string::npos)
        {
            str = str.substr(0, pos);
        }

        // Process str
        strData += str;
        strData.push_back('\n');
    }
    
    if(!settings.Parse(strData.c_str()))
    {
        printf("failed to parse json contents of file: %s\n", filename);
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// retrieve settings

const char* Config::GetStr(const char* setting, const char* defaultVal/*=NULL*/)
{
    const char* ret_val = settings.GetElemValue(setting);

    if(ret_val == NULL)
        return defaultVal;
    
    return ret_val;
}

int Config::GetInt(const char* setting, int unfoundVal/*=-1*/)
{
    return settings.GetElemInt(setting, unfoundVal);
}

float Config::GetFloat(const char* setting, float unfoundVal/*=0.0f*/)
{
    return settings.GetElemFloat(setting, unfoundVal);
}
