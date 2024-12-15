#ifndef POLIVMANAGER_H
#define POLIVMANAGER_H

#include <Arduino.h>
#include "config.h"
#include "PlantCatalog.h"
#include "Sensors.h"

/**
 * @brief Состояние каждого слота для растения.
 * SLOT_EMPTY: Нет растения
 * SLOT_SELECTED: Растение выбрано, но полив не идёт (влажность в норме или не требуется)
 * SLOT_WATERING: Идёт полив, влажность ниже минимальной
 * SLOT_ERROR: Истекло время ожидания, влажность не улучшилась
 */
enum SlotState {
    SLOT_EMPTY,
    SLOT_SELECTED,
    SLOT_WATERING,
    SLOT_ERROR
};

struct SlotData {
    String plantID;
    PlantConditions conditions;
    SlotState state;
    unsigned long wateringStartTime;
};

/**
 * @brief Класс для управления поливом.
 * 
 * Логика:
 * - Для каждого из 5 слотов можно выбрать растение (из каталога).
 * - Если влажность почвы ниже необходимой — включаем клапан и насос (SLOT_WATERING).
 * - Когда влажность достигнута — прекращаем полив (SLOT_SELECTED).
 * - Если за WATERING_TIMEOUT влажность не поднялась — SLOT_ERROR.
 * 
 * Взаимодействие:
 * - selectPlantForSlot() и clearPlantFromSlot() вызываются при выборе/очистке слота на веб-интерфейсе.
 * - update() вызывается раз в несколько секунд, обновляет состояние и управляет насосом/клапанами.
 */
class PolivManager {
public:
    PolivManager();

    bool begin(Sensors* s, PlantCatalog* pc);

    void update();

    bool selectPlantForSlot(int slotIndex, const String& plantID);
    bool clearPlantFromSlot(int slotIndex);

    SlotState getSlotState(int slotIndex) const;
    String getSlotPlantID(int slotIndex) const;

private:
    Sensors* sensors;
    PlantCatalog* plantCatalog;

    SlotData slots[SOIL_SENSOR_COUNT];

    unsigned long lastUpdate;

    void updateSlots();
    void controlValvesAndPump();
};

#endif // POLIVMANAGER_H
