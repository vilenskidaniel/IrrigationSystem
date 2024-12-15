#include "PlantCatalog.h"
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "config.h"

PlantCatalog::PlantCatalog() : plantCount(0) {
}

bool PlantCatalog::begin() {
    // Предполагаем, что LittleFS уже смонтирован в main.cpp.
    // Здесь только читаем JSON.
    if (!loadFromStorage()) {
        Serial.println("PlantCatalog: Failed to load from LittleFS!");
        return false;
    }
    return true;
}

bool PlantCatalog::loadFromStorage() {
    // Путь к файлу с каталогом
    // Определено в config.h, но лучше использовать константу
    // или просто указать строку: "/plants_catalog.json"
    // Предположим, что PLANTS_CATALOG_FILE = "/plants_catalog.json"
    if(!LittleFS.exists(PLANTS_CATALOG_FILE)) {
        Serial.printf("PlantCatalog: File %s not found in LittleFS\n", PLANTS_CATALOG_FILE);
        return false;
    }

    File file = LittleFS.open(PLANTS_CATALOG_FILE, "r");
    if(!file) {
        Serial.print("Failed to open ");
        Serial.println(PLANTS_CATALOG_FILE);
        return false;
    }

    // Предполагается большой JSON, используем DynamicJsonDocument
    DynamicJsonDocument doc(30000); // побольше, чтобы вместить 155 растений

    DeserializationError err = deserializeJson(doc, file);
    file.close();

    if (err) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(err.c_str());
        return false;
    }

    // Предполагается, что корневой элемент — массив объектов
    if(!doc.is<JsonArray>()) {
        Serial.println("PlantCatalog: Root is not an array!");
        return false;
    }

    JsonArray plants = doc.as<JsonArray>();
    if (plants.isNull()) {
        Serial.println("PlantCatalog: Empty array in JSON");
        return false;
    }

    plantCount = 0;

    // Перебираем растения
    for (JsonObject p : plants) {
        if (plantCount >= MAX_PLANTS) {
            Serial.println("MAX_PLANTS limit reached, some plants ignored.");
            break;
        }

        const char* pid = p["id"];
        if(!pid) {
            Serial.println("Plant without id found, skipping...");
            continue;
        }

        PlantInfo& info = plantList[plantCount];
        info.id = pid;

        JsonObject cond = p["optimalConditions"];
        if(cond.isNull()) {
            Serial.printf("No optimalConditions for plant %s\n", pid);
            // Если нет условий, пропускаем растение
            continue;
        }

        info.conditions.soilMoistureMin = cond["soilMoisture"]["min"] | 0;
        info.conditions.soilMoistureMax = cond["soilMoisture"]["max"] | 100;
        info.conditions.airTempMin = cond["airTemperature"]["min"] | 10;
        info.conditions.airTempMax = cond["airTemperature"]["max"] | 30;
        info.conditions.airHumidityMin = cond["airHumidity"]["min"] | 30;
        info.conditions.airHumidityMax = cond["airHumidity"]["max"] | 90;
        info.conditions.lightMin = cond["lightIntensity"]["min"] | 10000;
        info.conditions.lightMax = cond["lightIntensity"]["max"] | 50000;
        info.conditions.soilAcidityMin = cond["soilAcidity"]["min"] | 5.0;
        info.conditions.soilAcidityMax = cond["soilAcidity"]["max"] | 7.0;

        plantCount++;
    }

    Serial.printf("PlantCatalog: Loaded %d plants.\n", plantCount);
    return true;
}

const PlantInfo* PlantCatalog::getPlantByIndex(int index) const {
    if (index < 0 || index >= plantCount) return nullptr;
    return &plantList[index];
}

const PlantInfo* PlantCatalog::getPlantByID(const String& id) const {
    for (int i=0; i<plantCount; i++) {
        if (plantList[i].id == id) return &plantList[i];
    }
    return nullptr;
}
