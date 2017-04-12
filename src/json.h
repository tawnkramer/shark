#ifndef __JSON_H__
#define __JSON_H__

#include "jsmn/jsmn.h"

#define JSON_STR_LEN 256

/////////////////////////////////////////////////////////////////////
// Json
// simple json wrapper on jsmn that does not thrash memory under load.

class Json
{
public:

    Json(int maxTokens);
    ~Json();
    bool Parse(const char* str);
    const char* GetElemValue(const char* elemName);
    int GetElemInt(const char* elemName, int unfoundVal=-1);
    float GetElemFloat(const char* elemName, float unfoundVal=0.0f);

private:

    struct JsonElem {
        char name[JSON_STR_LEN];
        char value[JSON_STR_LEN];
    };


    int jsoneq(const char *json, jsmntok_t *tok, const char *s);

    int         m_maxTokens;
    int         m_numElem;    
    jsmn_parser m_Parser;
    jsmntok_t*  m_pTokens;
    JsonElem*   m_pElem;
};

#endif //__JSON_H__
