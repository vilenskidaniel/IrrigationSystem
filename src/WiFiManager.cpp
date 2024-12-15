// WiFiManager.cpp
#include "WiFiManager.h"
#include "config.h"

WiFiManager::WiFiManager() {
    memset(storedSSID, 0, sizeof(storedSSID));
    memset(storedPass, 0, sizeof(storedPass));
}

bool WiFiManager::begin() {
    if(!EEPROM.begin(EEPROM_SIZE)) {
        Serial.println("WiFiManager: Failed to initialise EEPROM");
        return false;
    }
    eepromInitialized = true;

    // Считаем сохранённые данные
    EEPROM.get(EEPROM_SSID_ADDR, storedSSID);
    EEPROM.get(EEPROM_PASS_ADDR, storedPass);

    return true;
}

bool WiFiManager::tryConnectSaved() {
    if (!eepromInitialized) {
        if(!begin()) {
            return false;
        }
    }

    if (strlen(storedSSID) == 0) {
        Serial.println("WiFiManager: No saved Wi-Fi credentials.");
        return false;
    }

    return connectToSaved();
}

bool WiFiManager::connectToSaved() {
    Serial.printf("WiFiManager: Connecting to WiFi: %s\n", storedSSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(storedSSID, storedPass);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFiManager: WiFi connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("WiFiManager: Failed to connect to saved WiFi");
        return false;
    }
}

void WiFiManager::saveCredentials(const char* ssid, const char* password) {
    if(!eepromInitialized) {
        begin();
    }

    strncpy(storedSSID, ssid, sizeof(storedSSID) - 1);
    storedSSID[sizeof(storedSSID) - 1] = '\0';

    strncpy(storedPass, password, sizeof(storedPass) - 1);
    storedPass[sizeof(storedPass) - 1] = '\0';

    EEPROM.put(EEPROM_SSID_ADDR, storedSSID);
    EEPROM.put(EEPROM_PASS_ADDR, storedPass);
    EEPROM.commit();

    Serial.println("WiFiManager: New credentials saved.");
}

void WiFiManager::startAPMode(const char* apName, const char* apPassword) {
    WiFi.mode(WIFI_AP);
    bool result = WiFi.softAP(apName, apPassword);
    if(result) {
        Serial.println("WiFiManager: AP Mode started.");
        Serial.print("Connect to: ");
        Serial.println(apName);
        Serial.print("AP IP: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("WiFiManager: Failed to start AP Mode.");
    }
}

bool WiFiManager::isConnected() {
    return (WiFi.status() == WL_CONNECTED);
}

String WiFiManager::getCurrentSSID() {
    if(isConnected()) {
        return WiFi.SSID();
    }
    return String(storedSSID);
}

IPAddress WiFiManager::getLocalIP() {
    if(isConnected()) {
        return WiFi.localIP();
    }
    return IPAddress(0,0,0,0);
}

void WiFiManager::initEEPROM() {
    if(!eepromInitialized) {
        if(!EEPROM.begin(EEPROM_SIZE)) {
            Serial.println("WiFiManager: Failed to init EEPROM");
        } else {
            eepromInitialized = true;
        }
    }
}
