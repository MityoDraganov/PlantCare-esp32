#include "driver.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <FS.h>
#include <SPIFFS.h>
#include <miniz.h>

const String BASE_ZIP_URL = "https://github.com/{owner}/{repo}/archive/refs/heads/{branch}.zip";

void DriverUtil::downloadDriver(const String &url, const String &filename)
{
    String zipUrl = constructZipUrl(url);
    if (!zipUrl.isEmpty())
    {
        downloadZip(zipUrl, filename);
        unzipFile(filename, filename); // Unzipping to a folder in SPIFFS
    }
    else
    {
        Serial.println("Failed to construct the ZIP URL.");
    }
}

String DriverUtil::constructZipUrl(const String &url)
{
    String zipUrl;
    if (url.indexOf("/tree/") > 0)
    {
        // If it's a folder, extract repo and branch info
        int repoEnd = url.indexOf("/tree/");
        String repoUrl = url.substring(0, repoEnd);
        int branchStart = repoEnd + 6; // "tree/" is 5 characters long
        int branchEnd = url.indexOf("/", branchStart);
        String branch = url.substring(branchStart, branchEnd);

        String ownerRepo = repoUrl.substring(19); // Extract {owner}/{repo} from the URL
        zipUrl = BASE_ZIP_URL;
        zipUrl.replace("{owner}", ownerRepo.substring(0, ownerRepo.indexOf("/")));
        zipUrl.replace("{repo}", ownerRepo.substring(ownerRepo.indexOf("/") + 1));
        zipUrl.replace("{branch}", branch);
    }
    else
    {
        // If it's a full repo, assume 'main' branch by default
        String ownerRepo = url.substring(19); // Extract {owner}/{repo} from the URL
        zipUrl = BASE_ZIP_URL;
        zipUrl.replace("{owner}", ownerRepo.substring(0, ownerRepo.indexOf("/")));
        zipUrl.replace("{repo}", ownerRepo.substring(ownerRepo.indexOf("/") + 1));
        zipUrl.replace("{branch}", "main"); // Default to main branch
    }

    return zipUrl;
}

void DriverUtil::downloadZip(const String &url, const String &filename)
{
    HTTPClient http;
    Serial.println("Downloading: " + url);
    http.begin(url);

    int httpCode = http.GET();
    Serial.println("HTTP Response Code: " + String(httpCode));

    if (httpCode == HTTP_CODE_OK)
    {
        File file = SPIFFS.open(filename, FILE_WRITE);
        if (file)
        {
            Serial.println("Writing file...");
            http.writeToStream(&file); // Directly write to the file stream
            file.close();
            Serial.println("Downloaded " + filename);
        }
        else
        {
            Serial.println("Failed to open file for writing: " + filename);
        }
    }
    else if (httpCode == HTTP_CODE_MOVED_PERMANENTLY || httpCode == HTTP_CODE_FOUND)
    {
        // Handle redirect
        String newLocation = http.getLocation();
        Serial.println("Redirected to: " + newLocation);
        http.end();              // Close the previous connection
        http.begin(newLocation); // Start a new connection with the redirected location
        httpCode = http.GET();   // Repeat the GET request

        if (httpCode == HTTP_CODE_OK)
        {
            File file = SPIFFS.open(filename, FILE_WRITE);
            if (file)
            {
                Serial.println("Writing file after redirect...");
                http.writeToStream(&file);
                file.close();
                Serial.println("Downloaded " + filename + " after redirect");
            }
            else
            {
                Serial.println("Failed to open file for writing after redirect: " + filename);
            }
        }
        else
        {
            Serial.println("Failed to download after redirect: " + newLocation);
        }
    }
    else
    {
        Serial.println("Failed to download: " + url + ". HTTP Code: " + String(httpCode));
    }

    http.end(); // Close the connection
}

void DriverUtil::unzipFile(const String &zipFilename, const String &destDir)
{
    // Open the ZIP file from SPIFFS
    File zipFile = SPIFFS.open(zipFilename, FILE_READ);
    if (!zipFile)
    {
        Serial.println("Failed to open ZIP file.");
        return;
    }

    // Read the ZIP file into a buffer
    size_t zipFileSize = zipFile.size();
    uint8_t *buffer = new uint8_t[zipFileSize];
    zipFile.read(buffer, zipFileSize);
    zipFile.close();

    mz_zip_archive zipArchive = {0};

    // Initialize the archive
    if (!mz_zip_reader_init_mem(&zipArchive, buffer, zipFileSize, 0))
    {
        Serial.println("Failed to initialize ZIP reader.");
        delete[] buffer;
        return;
    }

    // Extract all files
    for (mz_uint i = 0; i < mz_zip_reader_get_num_files(&zipArchive); ++i)
    {
        mz_zip_archive_file_stat fileStat;
        if (mz_zip_reader_file_stat(&zipArchive, i, &fileStat))
        {
            // Simplify the path to avoid directories in SPIFFS
            String filePath = destDir + "/" + String(fileStat.m_filename);

            // Remove directory components
            if (filePath.indexOf("/") >= 0)
            {
                filePath = filePath.substring(filePath.lastIndexOf("/") + 1);
            }

            File outFile = SPIFFS.open(filePath, FILE_WRITE);
            if (outFile)
            {
                // Allocate buffer to hold extracted data
                size_t fileSize = fileStat.m_uncomp_size;
                uint8_t *fileBuffer = new uint8_t[fileSize];
                if (mz_zip_reader_extract_to_mem(&zipArchive, i, fileBuffer, fileSize, 0))
                {
                    outFile.write(fileBuffer, fileSize);
                    Serial.println("Extracted: " + filePath);
                }
                else
                {
                    Serial.println("Failed to extract file: " + filePath);
                }
                delete[] fileBuffer;
                outFile.close();
            }
            else
            {
                Serial.println("Failed to open file for writing: " + filePath);
            }
        }
    }

    // Clean up
    mz_zip_reader_end(&zipArchive);
    delete[] buffer;
}
