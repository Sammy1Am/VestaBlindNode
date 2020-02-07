#include <Tracker.h>
namespace vesta {

#define NUM_DEVICES_TO_TRACK 2
#define VESTA_TRACKER_JSON_DOC_SIZE 107 + (34 * NUM_DEVICES_TO_TRACK)

uint8_t trackingFilter[NUM_DEVICES_TO_TRACK][MAC_ADDRESS_LENGTH] = {
    {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}, 
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05}};

// Since we know how many devices we're planning to track and what data we're including,
// we can use a Static document
StaticJsonDocument<VESTA_TRACKER_JSON_DOC_SIZE> trackerJsonDoc;
JsonObject s_wifi;

WiFiClient *trackerClient;
HTTPClient hc;


void Tracker::setup_tracker(WiFiClient *client) {
    trackerClient = client;
    initJson();
}

void Tracker::initJson() {
    s_wifi = trackerJsonDoc.createNestedObject("s").createNestedObject("wifi");
    trackerJsonDoc["d"] = VestaConfig::getConfig()->hostname;
    trackerJsonDoc["f"] = TRACKER_FAMILY_NAME;
}

void Tracker::spottedDevice(uint8_t macAddress[], signed int rssi) {
    char mac[18] = {0};
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);

    s_wifi[mac] = rssi;
}

bool Tracker::passesFilter(uint8_t macAddress[]) {
    for (int f = 0; f < NUM_DEVICES_TO_TRACK; f++) {
        bool filterPass = true;
        for (int p = 0; p < MAC_ADDRESS_LENGTH; p++) {
            if (trackingFilter[f][p] != macAddress[p]) {
                filterPass = false;
                break;
            }
        }
        if (filterPass) {
            return true;
            Serial.println("It passed!");
        }
    }
    return false;
}

bool Tracker::hasReportAvailable() {
    Serial.printf("Tracker has %u records to report.\n", s_wifi.size());
    return s_wifi.size() > 0;
}

void Tracker::sendReport() {
    char buffer[VESTA_TRACKER_JSON_DOC_SIZE];
    serializeJson(trackerJsonDoc, buffer);

    hc.begin(*trackerClient, VestaConfig::getConfig()->mqtt_server, 8005, "/passive");
    hc.addHeader("Content-Type", "application/json");
    int httpCode = hc.POST((uint8_t *)buffer, strlen(buffer));

    // httpCode will be negative on error
    if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] POST code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK) {
            const String &payload = hc.getString();
            Serial.print("server responded:\n<<");
            Serial.print(payload);
            Serial.println(">>");
        }
    } else {
        Serial.printf("[HTTP] POST failed, error: %s\n", hc.errorToString(httpCode).c_str());
    }

    hc.end();

    // Clear doc and release memory, then re-initialize
    trackerJsonDoc.clear();
    initJson();
}
} // namespace vesta
