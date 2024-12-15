// TurretManager.cpp
#include "TurretManager.h"

TurretManager::TurretManager() : turretAvailable(false) {
}

bool TurretManager::begin() {
    // Пока всегда false, т.к. турель не подключена.
    // Если в будущем турель подключена, можно установить turretAvailable = true
    return false;
}

void TurretManager::update() {
    // Пока ничего не делаем.
}

bool TurretManager::isAvailable() const {
    return turretAvailable;
}

bool TurretManager::aimAt(float azimuth, float elevation) {
    if (!turretAvailable) return false;
    // Реализация в будущем
    return false;
}

bool TurretManager::activateUltrasonic() {
    if (!turretAvailable) return false;
    return false;
}

bool TurretManager::activateStrobe() {
    if (!turretAvailable) return false;
    return false;
}

bool TurretManager::activateWaterDefense() {
    if (!turretAvailable) return false;
    return false;
}

bool TurretManager::stopAllDefenses() {
    if (!turretAvailable) return false;
    return false;
}
