#pragma once

const char* configJson = R"rawliteral(
{
    "sensors": [
        {
            "serialNumber": "YKTMgxAKCwE5jNXo",
            "type": "MoistureSensor"
        },
        {
            "serialNumber": "oMaRjWZ554jdMf2Y",
            "name": "TemperatureSensor"
        }
    ],
    "controls": [
        {
            "serialNumber": "",
            "type": "WaterPump",
            "dependantSensor": {
                "serialNumber": "YKTMgxAKCwE5jNXo",
                "minValue": 0,
                "maxValue": 50
            }
        }
    ]
}
)rawliteral";
