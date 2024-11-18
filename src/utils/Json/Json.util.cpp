#include <ArduinoJson.h>
#include "utils/Json/Json.util.h"


void JsonUtil::printJson(const JsonVariant &json, int indent)
{
    // Create the indentation string manually
    String indentStr;
    for (int i = 0; i < indent; i++)
    {
        indentStr += "  "; // Adds two spaces per level
    }

    if (json.is<JsonObject>())
    {
        for (JsonPair kv : json.as<JsonObject>())
        {
            Serial.print(indentStr);
            Serial.print(kv.key().c_str());
            Serial.print(": ");
            printJson(kv.value(), indent + 1); // Recursively print nested objects
        }
    }
    else if (json.is<JsonArray>())
    {
        for (JsonVariant v : json.as<JsonArray>())
        {
            printJson(v, indent + 1);
        }
    }
    else if (json.is<const char *>())
    {
        Serial.println(json.as<const char *>());
    }
    else if (json.is<int>())
    {
        Serial.println(json.as<int>());
    }
    else if (json.is<float>())
    {
        Serial.println(json.as<float>());
    }
    else if (json.is<bool>())
    {
        Serial.println(json.as<bool>() ? "true" : "false");
    }
    else
    {
        Serial.println("null");
    }
}
