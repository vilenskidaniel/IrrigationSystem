#include "Sensors.h"
#include <Wire.h>

Sensors::Sensors() :
    dht(DHT_PIN, DHT_TYPE),
    temperature(NAN),
    humidity(NAN),
    lux(-1),
    lastReadTime(0),
    initialized(false)
{
    for (int i=0; i<SOIL_SENSOR_COUNT; i++) {
        soilMoisture[i] = -1;
    }
}

bool Sensors::begin() {
    // Инициализация DHT
    dht.begin();

    // Инициализация I2C для BH1750
    Wire.begin(BH1750_SDA, BH1750_SCL);
    if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
        Serial.println("BH1750 init failed!");
        // Можно вернуть false, чтобы main знал о проблеме
        return false;
    }

    // Аналоговые датчики почвы не требуют специальной инициализации
    // Просто будем читать analogRead(index) когда нужно.

    initialized = true;
    // Прочитаем начальные значения
    readDHT();
    readBH1750();
    readSoil();
    return true;
}

void Sensors::update() {
    if (!initialized) return; // Если датчики не инициализированы

    unsigned long now = millis();
    if (now - lastReadTime > SENSORS_READ_INTERVAL) {
        lastReadTime = now;
        readDHT();
        readBH1750();
        readSoil();
    }
}

void Sensors::readDHT() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (isnan(t) || isnan(h)) {
        Serial.println("Failed to read from DHT!");
        // Не обновляем значения, оставляем старые
    } else {
        temperature = t;
        humidity = h;
    }
}

void Sensors::readBH1750() {
    float l = lightMeter.readLightLevel();
    if (l < 0) {
        Serial.println("Failed to read from BH1750!");
        // Не обновляем lux
    } else {
        lux = l;
    }
}

void Sensors::readSoil() {
    // Аналоговый вход: val 0..4095 для ESP32 (12 bit)
    // Мы мапируем 0..100%
    // Если датчик отсутствует или не втыкнут — может быть неверное значение
    // Но мы считаем, что если analogRead вернёт значение, просто мапируем.
    // Если нужно, можно добавить проверку корректности.
    for (int i=0; i<SOIL_SENSOR_COUNT; i++) {
        int val = analogRead(SOIL_PINS[i]);
        float moisture = map((long)val, 4095, 0, 0, 100);
        if (moisture < 0) moisture = 0;
        if (moisture > 100) moisture = 100;
        soilMoisture[i] = moisture;
    }
}
