#include "Network.h"
#include "Ticker.h"
#include "VestaConfig.h"

using namespace vesta;

WiFiClient sharedNetworkClient;

#ifdef USE_SENSOR
#include "Sensor.h"
void sensorTick();
Ticker sensingTicker(sensorTick, VESTA_SENSOR_MAX_INTERVAL, 0, MILLIS);
#endif

#ifdef USE_TRACKER
#include "Tracker.h"
void trackerTick();
Ticker trackingTicker(trackerTick, 5000, 0, MILLIS);
#endif

void setup() {
    Serial.begin(VESTA_BAUDRATE);
    VestaConfig::setup_config(); // Setup configuration first to configure / so we have access to it during setup

    // Setup sensor and tracker
#ifdef USE_SENSOR
    Sensor::setup_sensor(&sharedNetworkClient);
#endif

#ifdef USE_TRACKER
    Tracker::setup_tracker(&sharedNetworkClient);
#endif

    // Connect to the network first to get channel to listen on
    vesta::Network::connect();
    delay(500);

    // Switch to listen mode for tracking
#ifdef USE_TRACKER
    vesta::Network::listen();
#endif

    // Start tickers to report packets and publish sensor data
#ifdef USE_SENSOR
    sensingTicker.start();
#endif
#ifdef USE_TRACKER
    trackingTicker.start();
#endif
}

void loop() {
    // Update tracking first since it may run the sensor report and we can skip the sensing tick.
#ifdef USE_TRACKER
    trackingTicker.update();
#endif
#ifdef USE_SENSOR
    sensingTicker.update();
#endif

    VestaConfig::readSerial();
}

// Called periodically to ask the tracker to report any new findings
void trackerTick() {
    // If we have a report, try switching to CONNECT (if it fails, it's because we're already connected, don't run twice!)
    if (Tracker::hasReportAvailable() && Network::connect()) {
        Tracker::sendReport();

#ifdef USE_SENSOR
        // If at least enough time has passed to do a sensor report, do it and restart the ticker
        if (sensingTicker.elapsed() > VESTA_SENSOR_MIN_INTERVAL) {
            Sensor::sendReport();
            sensingTicker.start();
        }
#endif

        Network::listen(); // We're all done, go back to listening
    }
}

#ifdef USE_SENSOR
void sensorTick() {
    if (Network::connect()) {
        Sensor::sendReport();
#ifdef USE_TRACKER
        Network::listen();
#endif
    }
}
#endif