#ifndef DRIVER_H
#define DRIVER_H

#include <FS.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
#include <Update.h>

class DriverUtil {
public:
    DriverUtil();
    bool handleUpdate(uint8_t* data, size_t len);
    bool endUpdate();

private:
    bool _updateStarted;
    size_t _totalBytes;
    size_t _receivedBytes;
};

#endif // DRIVER_H
