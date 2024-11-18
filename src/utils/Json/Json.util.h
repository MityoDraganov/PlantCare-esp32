#ifndef JSON_UTIL_H
#define JSON_UTIL_H

#include <Arduino.h>

class JsonUtil
{
public:
    void printJson(const JsonVariant &json, int indent);
};

#endif