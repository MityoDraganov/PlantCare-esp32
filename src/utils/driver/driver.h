#ifndef DRIVER_H
#define DRIVER_H

#include "driver.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <FS.h>
#include <SPIFFS.h>
#include <miniz.h>

class DriverUtil {
public:
    static void downloadDriver(const String &url, const String &filename);
    static void unzipFile(const String &zipFilename, const String &destDir);

private:
    static String constructZipUrl(const String &url);
    static void downloadZip(const String &url, const String &filename);
};

#endif // DRIVER_H
