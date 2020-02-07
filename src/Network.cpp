#include "Network.h"

namespace vesta {

Network::NetworkMode Network::currentMode = NetworkMode::STOPPED;
const char *ssid = VestaConfig::getConfig()->wifi_ssid;
const char *password = VestaConfig::getConfig()->wifi_psk;
int Network::homeChannel = 0; // Start on 0 (to indicate it's uninitialized), but write this as soon as we're connected
uint8_t *homeBSSID;

bool Network::connect() {
    // If this is called when we're already connected, or in the process of starting to listen, do nothing

    if (currentMode == LISTEN) {
        stopListen();
    }

    if (currentMode != STOPPED) {
        return false;
    }

    currentMode = START_CONNECT;

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    if (WiFi.getMode() != WIFI_STA){
        WiFi.mode(WIFI_STA);
    }

    
    if (homeChannel > 0) {
        WiFi.begin(ssid, password, homeChannel, homeBSSID);
    } else {
        // These will happen only the first time
        WiFi.hostname(VestaConfig::getConfig()->hostname);
        WiFi.begin(ssid, password);
    }
    

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.print("\n");

    homeChannel = WiFi.channel();
    homeBSSID = WiFi.BSSID();

    currentMode = CONNECT;

    return true;
}

void Network::stopConnect() {
    WiFi.disconnect();
    Serial.println("Disconnecting from WiFi");
    while (WiFi.status() != WL_IDLE_STATUS) {
        delay(10);
        Serial.print(".");
    }
    Serial.print("\n");
    currentMode = STOPPED;
}

bool Network::listen() {
    if (currentMode == CONNECT) {
        stopConnect();
    }

    if (currentMode != STOPPED) {
        return false;
    }

    currentMode = START_LISTEN;

    Serial.print("Entering listen mode...");

    wifi_set_opmode(STATION_MODE);
    wifi_set_channel(homeChannel);
    wifi_promiscuous_enable(0);
    delay(10);
    wifi_set_promiscuous_rx_cb(sniffer_callback);
    delay(10);
    wifi_promiscuous_enable(1);

    currentMode = LISTEN;
    Serial.println("Listening.");
    //WiFi.printDiag(Serial);
    return true;
}

void Network::stopListen() {
    wifi_promiscuous_enable(0);
    currentMode = STOPPED;
}

// Callback for received packets
void ICACHE_FLASH_ATTR Network::sniffer_callback(uint8_t *buffer, uint16_t length) {
    struct SnifferPacket *snifferPacket = (struct SnifferPacket *)buffer;
    if (Tracker::passesFilter(snifferPacket->data + 10)) {
        Tracker::spottedDevice(snifferPacket->data + 10, snifferPacket->rx_ctrl.rssi);
    }
}
}