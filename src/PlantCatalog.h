#ifndef PLANTCATALOG_H
#define PLANTCATALOG_H

#include "config.h"
#include <Arduino.h>

/**
 * @brief Структура условий выращивания растения.
 * Вся информация берётся из JSON каталога.
 */
struct PlantConditions {
    float soilMoistureMin;
    float soilMoistureMax;
    float airTempMin;
    float airTempMax;
    float airHumidityMin;
    float airHumidityMax;
    float lightMin;
    float lightMax;
    float soilAcidityMin;
    float soilAcidityMax;
};

/**
 * @brief Информация о растении: ID и оптимальные условия.
 * В будущем можно расширить: добавить описания, имена на разных языках, список изображений.
 */
struct PlantInfo {
    String id;
    PlantConditions conditions;
    // Можно добавить поля name, description, images при необходимости, 
    // но для логики полива нужны только conditions.
};

class PlantCatalog {
public:
    PlantCatalog();

    /**
     * @brief Инициализация каталога.
     * Предполагается, что `plants_catalog.json` хранится в LittleFS.
     * Возвращает true при успешной загрузке.
     */
    bool begin(); 

    int getPlantCount() const { return plantCount; }

    /**
     * @brief Получить растение по индексу.
     */
    const PlantInfo* getPlantByIndex(int index) const;

    /**
     * @brief Получить растение по ID.
     */
    const PlantInfo* getPlantByID(const String& id) const;

    // В будущем можно добавить методы фильтрации, поиска по названию и т.д.

private:
    bool loadFromStorage();

    PlantInfo plantList[MAX_PLANTS];
    int plantCount;
};

#endif // PLANTCATALOG_H
