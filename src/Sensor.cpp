#include <Sensor.h>

namespace vesta {

BME280 bmeSensor;
#define VESTA_SENSOR_JSON_DOC_SIZE 78
StaticJsonDocument<VESTA_SENSOR_JSON_DOC_SIZE> sensorJsonDoc;
//PubSubClient* mqttClient;
MQTTClient mqttClient;

char sensorTopic[52];

void Sensor::setup_sensor(Client *client) {
    mqttClient.begin(VestaConfig::getConfig()->mqtt_server, *client);
    sprintf(sensorTopic, "environment/sensor/%s", VestaConfig::getConfig()->hostname);

    Wire.begin(VESTA_I2C_SDA, VESTA_I2C_SDL);

    bmeSensor.settings.I2CAddress = VESTA_I2C_BME280_ADDR;
    //bmeSensor.setTemperatureCorrection(-2.0f);
    bmeSensor.beginI2C();
    bmeSensor.setStandbyTime(5);
    bmeSensor.setMode(MODE_SLEEP);
    bmeSensor.setHumidityOverSample(1);
    bmeSensor.setPressureOverSample(1);
    bmeSensor.setTempOverSample(1);
}

void Sensor::takeForcedReading() {
    bmeSensor.setMode(MODE_FORCED); //Wake up sensor and take reading
    while (bmeSensor.isMeasuring() == false)
        ; //Wait for sensor to start measurment
    while (bmeSensor.isMeasuring() == true)
        ; //Hang out while sensor completes the reading
    
    // Update json
    sensorJsonDoc["pressure"] = bmeSensor.readFloatPressure();
    sensorJsonDoc["humidity"] = bmeSensor.readFloatHumidity();
    sensorJsonDoc["temperature"] = bmeSensor.readTempF();
}

void Sensor::sendReport() {
    takeForcedReading();

    if (mqttClient.connect(VestaConfig::getConfig()->mqtt_server)){
        char buffer[VESTA_SENSOR_JSON_DOC_SIZE];
        serializeJson(sensorJsonDoc, buffer);
        mqttClient.publish(sensorTopic, buffer, true, 0);
        serializeJson(sensorJsonDoc, Serial);
        Serial.println(" sent to server");
        mqttClient.loop(); // Call once just in case there's some cleanup or something here...
        mqttClient.disconnect();
    } else {
        Serial.printf("MQTT Connection failed (%d)\n", mqttClient.lastError());
    }
}
}

