#ifndef SENSOR_H
#define SENSOR

#include "VestaConfig.h"
#include <SparkFunBME280.h>
#include <ArduinoJson.h>
#include <MQTT.h>

#define VESTA_SENSOR_MIN_INTERVAL 60000 // One minute minimum (no need to report more often than that)
#define VESTA_SENSOR_MAX_INTERVAL 240000 // Four minute maximum (report at *least* this often)

#define VESTA_I2C_SDA 12
#define VESTA_I2C_SDL 14
#define VESTA_I2C_BME280_ADDR 0x76

namespace vesta {
class Sensor {
public:
    static void setup_sensor(Client* client);
    static void sendReport();

private:
    Sensor(/* args */){};
    static void takeForcedReading();
};

} // namespace vesta

#endif