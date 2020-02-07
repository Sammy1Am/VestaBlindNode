#include <VestaConfig.h>

namespace vesta {

VestaConfiguration config;
char receivedCharacters[32];

void VestaConfig::setup_config() {
    EEPROM.begin(sizeof(config) + 1);

    // If a previous config exists, retrieve it.
    if (EEPROM.read(0x00) == 'V') {
        EEPROM.get(0x01, config);
    } else {
        reconfigure(); //Otherwise, immediately enter configuration
    }

    Serial.println("Press Ctrl+[ for interactive prompt.");
}

void VestaConfig::readSerial() {
    if (Serial.available() > 0 && Serial.read() == 27) {
        Serial.print(">");
        blockingSerialRead(receivedCharacters, sizeof(receivedCharacters));

        if (strcmp(receivedCharacters, "reconfigure") == 0) {
            reconfigure();
        }
    }
}

void VestaConfig::blockingSerialRead(char buffer[], int bufferLength) {
    uint bufferPos = 0;
    char c;
    bool doneReading = false;

    // Block until we're done reading (this stops all other processes!)
    while (!doneReading) {
        while (Serial.available() > 0) {
            c = Serial.read();

            if (c == '\r') {
                continue; // Eat weird Windows line returns
            }

            if (c == '\n') {
                // If it's a newline, we're done.  Terminate the string and break.
                buffer[bufferPos] = '\0';
                doneReading = true;
                break;
            } else if (bufferPos < (bufferLength - 1u)) {
                // Otherwise (only if we have buffer space left), echo the key and record it.
                Serial.write(c);
                buffer[bufferPos] = c;
                bufferPos++;
            }
        }
    }
}

void VestaConfig::reconfigure() {
    // Get new config info
    Serial.println("Enter new configuration info.  Press 'enter' to keep existing value.  Enter '-' to set empty string.");
    promptForValue("Hostname", config.hostname, sizeof(config.hostname));
    promptForValue("WiFi SSID", config.wifi_ssid, sizeof(config.wifi_ssid));
    promptForValue("WiFi Password", config.wifi_psk, sizeof(config.wifi_psk));
    promptForValue("MQTT Host", config.mqtt_server, sizeof(config.mqtt_server));
    promptForValue("MQTT Username", config.mqtt_user, sizeof(config.mqtt_user));
    promptForValue("MQTT Password", config.mqtt_pass, sizeof(config.mqtt_pass));

    // Write to EEPROM
    EEPROM.write(0x00, 'V'); // Write config bit
    EEPROM.put(0x01, config);
    EEPROM.commit();
}

void VestaConfig::promptForValue(const char friendlyName[], char value[], uint maxValueLength) {
    char userInput[32];
    Serial.printf("\n%s[%d](%s):", friendlyName, maxValueLength, value);
    blockingSerialRead(userInput, 32);
    if (strlen(userInput) > 0 && strlen(userInput) <= maxValueLength) {
        if (userInput[0] == '-') {
            value[0] = '\0'; // If the first character is a hyphen, treat this as an empty string.
        } else {
            strcpy(value, userInput);
        }
    }
}

VestaConfiguration *VestaConfig::getConfig() {
    return &config;
}
} // namespace vesta