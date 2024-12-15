// TurretManager.h
#ifndef TURRETMANAGER_H
#define TURRETMANAGER_H

#include <Arduino.h>

/**
 * @brief Класс TurretManager для управления турелью.
 * Пока турель недоступна, все методы возвращают false.
 * 
 * В будущем:
 * - Реализовать логику подключения к Raspberry Pi.
 * - Отправка команд (наведение, включение ультразвука, стробоскопа, воды и т.д.)
 * - Получение статуса.
 * 
 * isAvailable() показывает, подключена ли турель. Сейчас always false.
 */
class TurretManager {
public:
    TurretManager();
    bool begin();
    void update();
    bool isAvailable() const;

    bool aimAt(float azimuth, float elevation);
    bool activateUltrasonic();
    bool activateStrobe();
    bool activateWaterDefense();
    bool stopAllDefenses();

private:
    bool turretAvailable;
};

#endif // TURRETMANAGER_H
