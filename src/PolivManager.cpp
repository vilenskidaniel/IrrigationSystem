#include "PolivManager.h"
#include "config.h"

PolivManager::PolivManager() : sensors(nullptr), plantCatalog(nullptr), lastUpdate(0)
{
    for (int i=0; i<SOIL_SENSOR_COUNT; i++) {
        slots[i].plantID = "";
        slots[i].state = SLOT_EMPTY;
        slots[i].wateringStartTime = 0;
    }
}

bool PolivManager::begin(Sensors* s, PlantCatalog* pc) {
    if (!s || !pc) {
        Serial.println("PolivManager: Null pointer in begin()");
        return false;
    }
    sensors = s;
    plantCatalog = pc;

    // Инициализация насосов и клапанов
    pinMode(PUMP_PIN, OUTPUT);
    digitalWrite(PUMP_PIN, LOW);

    for (int i=0; i<VALVE_COUNT; i++) {
        pinMode(VALVE_PINS[i], OUTPUT);
        digitalWrite(VALVE_PINS[i], LOW);
    }

    return true;
}

void PolivManager::update() {
    unsigned long now = millis();
    if (now - lastUpdate > POLIV_UPDATE_INTERVAL) {
        lastUpdate = now;
        updateSlots();
        controlValvesAndPump();
    }
}

bool PolivManager::selectPlantForSlot(int slotIndex, const String& plantID) {
    if (slotIndex < 0 || slotIndex >= SOIL_SENSOR_COUNT) {
        Serial.println("PolivManager: Invalid slot index in selectPlantForSlot");
        return false;
    }
    const PlantInfo* pInfo = plantCatalog->getPlantByID(plantID);
    if(!pInfo) {
        Serial.printf("PolivManager: Plant ID %s not found in catalog\n", plantID.c_str());
        return false;
    }

    slots[slotIndex].plantID = plantID;
    slots[slotIndex].conditions = pInfo->conditions;
    slots[slotIndex].state = SLOT_SELECTED;
    slots[slotIndex].wateringStartTime = 0;
    Serial.printf("PolivManager: Slot %d assigned to plant %s\n", slotIndex, plantID.c_str());

    return true;
}

bool PolivManager::clearPlantFromSlot(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= SOIL_SENSOR_COUNT) {
        return false;
    }
    slots[slotIndex].plantID = "";
    slots[slotIndex].state = SLOT_EMPTY;
    slots[slotIndex].wateringStartTime = 0;
    Serial.printf("PolivManager: Slot %d cleared.\n", slotIndex);
    return true;
}

SlotState PolivManager::getSlotState(int slotIndex) const {
    if (slotIndex < 0 || slotIndex >= SOIL_SENSOR_COUNT) {
        return SLOT_ERROR;
    }
    return slots[slotIndex].state;
}

String PolivManager::getSlotPlantID(int slotIndex) const {
    if (slotIndex < 0 || slotIndex >= SOIL_SENSOR_COUNT) return "";
    return slots[slotIndex].plantID;
}

void PolivManager::updateSlots() {
    for (int i=0; i<SOIL_SENSOR_COUNT; i++) {
        SlotData &slot = slots[i];
        float currentMoisture = sensors->getSoilMoisture(i);
        unsigned long now = millis();

        switch(slot.state) {
            case SLOT_EMPTY:
                // Нет растения, ничего не делаем
                break;

            case SLOT_SELECTED:
                // Проверяем, нужна ли вода
                if (currentMoisture >= 0 && currentMoisture < slot.conditions.soilMoistureMin) {
                    slot.state = SLOT_WATERING;
                    slot.wateringStartTime = now;
                    Serial.printf("Slot %d: moisture=%.1f%% < %.1f%% min, start watering\n", i, currentMoisture, slot.conditions.soilMoistureMin);
                }
                break;

            case SLOT_WATERING:
                // Проверяем, достигли ли нужной влажности
                if (currentMoisture >= slot.conditions.soilMoistureMin) {
                    slot.state = SLOT_SELECTED;
                    slot.wateringStartTime = 0;
                    Serial.printf("Slot %d: moisture=%.1f%% >= %.1f%% min, stop watering\n", i, currentMoisture, slot.conditions.soilMoistureMin);
                } else {
                    // Ещё недостаточно влажности, проверяем таймаут
                    unsigned long elapsed = now - slot.wateringStartTime;
                    if (elapsed > WATERING_TIMEOUT) {
                        slot.state = SLOT_ERROR;
                        Serial.printf("Slot %d: watering timeout reached! Still %.1f%% < %.1f%%. ERROR\n", i, currentMoisture, slot.conditions.soilMoistureMin);
                        // При желании можно вызвать notifier.notifyPolivError(i);
                    }
                }
                break;

            case SLOT_ERROR:
                // Ошибка, ручное вмешательство необходимо.
                break;
        }
    }
}

void PolivManager::controlValvesAndPump() {
    bool anyWatering = false;

    for (int i=0; i<SOIL_SENSOR_COUNT && i<VALVE_COUNT; i++) {
        if (slots[i].state == SLOT_WATERING) {
            anyWatering = true;
            digitalWrite(VALVE_PINS[i], HIGH);
        } else {
            digitalWrite(VALVE_PINS[i], LOW);
        }
    }

    // Насос включаем, если есть хоть один слот в состоянии WATERING
    if (anyWatering) {
        digitalWrite(PUMP_PIN, HIGH);
    } else {
        digitalWrite(PUMP_PIN, LOW);
    }
}
