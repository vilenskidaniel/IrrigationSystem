#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <DHT.h>
#include <BH1750.h>
#include "config.h"

/**
 * @brief Класс для работы с датчиками:
 * - DHT22 (температура и влажность воздуха)
 * - BH1750 (освещённость)
 * - Ёмкостные датчики влажности почвы (аналоговые входы)
 * 
 * Логика:
 * - begin() инициализирует датчики.
 * - update() вызывается раз в SENSORS_READ_INTERVAL мс, обновляет значения.
 * - Если чтение не удалось, оставляем старое значение.
 * 
 * Доступ к данным через getTemperature(), getHumidity(), getLux(), getSoilMoisture().
 * Если датчик почвы отсутствует или выдает некорректные данные, moisture = -1 или прочерк.
 */
class Sensors {
public:
    Sensors();
    bool begin();    // Инициализация всех датчиков
    void update();   // Периодический опрос датчиков

    float getTemperature() const { return temperature; }
    float getHumidity() const { return humidity; }
    float getLux() const { return lux; }
    float getSoilMoisture(int index) const {
        if (index < 0 || index >= SOIL_SENSOR_COUNT) return -1;
        return soilMoisture[index];
    }

private:
    DHT dht;           // DHT объект
    BH1750 lightMeter; // BH1750 объект
    float temperature;
    float humidity;
    float lux;
    float soilMoisture[SOIL_SENSOR_COUNT];

    unsigned long lastReadTime;
    bool initialized;

    void readDHT();
    void readBH1750();
    void readSoil();
};

#endif // SENSORS_H
