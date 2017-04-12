#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json.h"

int min(int a, int b)
{
    return a < b ? a : b;
}

/////////////////////////////////////////////////////////////////////
// Json
// simple json wrapper on jsmn that does not thrash memory under load.

Json::Json(int maxTokens)
{
    m_pTokens = new jsmntok_t[maxTokens];
    m_maxTokens = maxTokens;
    m_numElem = 0;
    m_pElem = new JsonElem[m_maxTokens];
}

Json::~Json()
{
    delete[] m_pTokens;
}

int Json::jsoneq(const char *json, jsmntok_t *tok, const char *s) 
{
    if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
            strncmp(json + tok->start, s, tok->end - tok->start) == 0) 
    {
        return 0;
    }

    return -1;
}

bool Json::Parse(const char* str)
{
    jsmn_init(&m_Parser);
    m_numElem = 0;

    int r = jsmn_parse(&m_Parser, str, strlen(str), m_pTokens, m_maxTokens);
    int len;

    if (r > 0)
    {
        if (r < 1 || m_pTokens[0].type != JSMN_OBJECT) 
        {
            printf("Object expected\n");
            return false;
        }

        for (int i = 1; (i + 1) < r && m_numElem < m_maxTokens; i++)
        {
            JsonElem& elem = m_pElem[m_numElem++];

            jsmntok_t& t = m_pTokens[i];
            len = min(t.end - t.start, JSON_STR_LEN);
            strncpy(elem.name, str + t.start, len);
            elem.name[len] = 0;

            jsmntok_t& v = m_pTokens[i + 1];
            len = min(v.end - v.start, JSON_STR_LEN);
            strncpy(elem.value, str + v.start, len);
            elem.value[len] = 0;          

            i++;
        }
    }

    return r > 0;
}

const char* Json::GetElemValue(const char* elemName)
{
    for(int iElem = 0; iElem < m_numElem; iElem++)
    {
        if(strcmp(elemName, m_pElem[iElem].name) == 0)
        {
            return m_pElem[iElem].value;
        }
    }

    return NULL;
}

int Json::GetElemInt(const char* elemName, int unfoundVal)
{
    int ret = unfoundVal;

    const char* value = GetElemValue(elemName);

    if(value != NULL)
    {
        ret = atoi(value);
    }

    return ret;
}

float Json::GetElemFloat(const char* elemName, float unfoundVal)
{
    float ret = unfoundVal;

    const char* value = GetElemValue(elemName);

    if(value != NULL)
    {
        ret = atof(value);
    }

    return ret;
}
