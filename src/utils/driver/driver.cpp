#include "driver.h"

DriverUtil::DriverUtil() : _updateStarted(false), _totalBytes(0), _receivedBytes(0) {}

bool DriverUtil::handleUpdate(uint8_t* data, size_t len) {
    if (!_updateStarted) {
        if (Update.begin(_totalBytes)) {
            _updateStarted = true;
        } else {
            Serial.println("Not enough space for OTA");
            return false;
        }
    }

    if (Update.write(data, len) != len) {
        Serial.println("Write failed");
        return false;
    }

    _receivedBytes += len;
    if (_receivedBytes >= _totalBytes) {
        return endUpdate();
    }

    return true;
}

bool DriverUtil::endUpdate() {
    if (_updateStarted) {
        if (Update.end()) {
            _updateStarted = false;
            return true;
        } else {
            Serial.println("Update failed");
        }
    }
    return false;
}
