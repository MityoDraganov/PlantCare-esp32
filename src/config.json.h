#pragma once

const char* configJson = R"rawliteral(
{
    "sensors": [
        {
            "serialNumber": "YKTMgxAKCwE5jNXo",
            "type": "MoistureSensor"
        }
    ],
    "controls": [
        {
            "serialNumber": "aG4nx27foxCW0N5q",
            "type": "ValveControl",
            "dependantSensor": {
                "serialNumber": "YKTMgxAKCwE5jNXo",
                "minValue": 0,
                "maxValue": 50
            }
        }
    ]
}
)rawliteral";
