#ifndef VESTACONFIG_H
#define VESTACONFIG_H

#include <EEPROM.h>
#include <Arduino.h>

#define VESTA_BAUDRATE 115200

#define USE_SENSOR
#define USE_TRACKER

namespace vesta {

    struct VestaConfiguration {
        char hostname[32];
        char wifi_ssid[16];
        char wifi_psk[16];
        char mqtt_server[32];
        char mqtt_user[16];
        char mqtt_pass[16];
    };

    class VestaConfig {
    public:
        static void readSerial();
        static void setup_config();
        static void reconfigure();
        static VestaConfiguration* getConfig();

    private:
        static void blockingSerialRead(char buffer[], int bufferLength);
        static void promptForValue(const char friendlyName[], char value[], uint maxValueLength);
        VestaConfig(/* args */){};
    };
    
}

#endif