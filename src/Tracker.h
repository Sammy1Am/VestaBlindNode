#ifndef TRACKER_H
#define TRACKER_H

#include "Network.h"
#include "VestaConfig.h"
#include <Arduino.h>
#include <ESP8266HTTPClient.h>

#define MAC_ADDRESS_LENGTH 6
#define TRACKER_FAMILY_NAME "TFamilyName" //TODO Genericize this

#define ARDUINOJSON_USE_LONG_LONG 1 // we need to store long long
#include <ArduinoJson.h>

namespace vesta {
class Tracker {
public:
    static void setup_tracker(WiFiClient* client);
    /// Called with signal info when a device is spotted
    static void spottedDevice(uint8_t macAddress[], signed int rssi); 
    /// Checks tracking filters so we only track devices we care about
    static bool passesFilter(uint8_t macAddress[]);
    /// Returns true if a device has been seen since the last report was sent.
    static bool hasReportAvailable();
    /// Sends latest device info to server
    static void sendReport(); 

private:
    Tracker(){};
    static void initJson();
};

} // namespace vesta

#endif