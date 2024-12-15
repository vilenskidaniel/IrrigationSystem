// WiFiManager.h
#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>

/**
 * @brief Класс для управления Wi-Fi подключением:
 * - Чтение/запись логина и пароля в EEPROM
 * - Подключение к сохранённой сети
 * - Запуск точки доступа, если нет данных или подключение не удалось
 * 
 * Использование:
 * 1. begin() - инициализация EEPROM
 * 2. tryConnectSaved() - попытка подключиться к сохранённой сети
 * 3. startAPMode() - запуск AP для Captive Portal
 * 
 * При сохранении новых данных Wi-Fi вызываем saveCredentials().
 * 
 * Примечание: Captive Portal должен настраиваться в main.cpp, здесь только логика Wi-Fi.
 */
class WiFiManager {
public:
    WiFiManager();
    bool begin(); // Инициализация EEPROM
    bool tryConnectSaved(); // Попытка подключиться к сохранённой сети

    void saveCredentials(const char* ssid, const char* password);

    void startAPMode(const char* apName, const char* apPassword = "");

    bool isConnected();
    String getCurrentSSID();
    IPAddress getLocalIP();

private:
    bool eepromInitialized = false;
    char storedSSID[32];
    char storedPass[64];

    void initEEPROM();
    bool connectToSaved();
};
