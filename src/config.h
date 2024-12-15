#ifndef CONFIG_H
#define CONFIG_H

// EEPROM настройки для хранения Wi-Fi
#define EEPROM_SIZE 512
#define EEPROM_SSID_ADDR 0
#define EEPROM_PASS_ADDR 100

// Настройки датчиков
#define DHT_PIN 32
#define DHT_TYPE 22 // DHT22
#define BH1750_SDA 21
#define BH1750_SCL 22

// Почвенные датчики (ёмкостные)
#define SOIL_SENSOR_COUNT 5
static const int SOIL_PINS[SOIL_SENSOR_COUNT] = {33,34,35,36,39};

// Интервалы обновления датчиков
#define SENSORS_READ_INTERVAL 5000
#define POLIV_UPDATE_INTERVAL 5000

// Настройки полива
#define PUMP_PIN 26
#define VALVE_COUNT 5
static const int VALVE_PINS[VALVE_COUNT] = {27, 14, 12, 13, 25};
#define WATERING_TIMEOUT 60000 // 1 минута ожидания

// Файл каталога растений
#define PLANTS_CATALOG_FILE "/plants_catalog.json"
#define MAX_PLANTS 200

// Настройки Telegram
#define TELEGRAM_ENABLED true
#define TELEGRAM_TOKEN "123456789:ABC-YourTokenHere"
#define TELEGRAM_CHAT_ID "123456789" 

#endif // CONFIG_H
