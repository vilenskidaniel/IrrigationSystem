// WebUI.h
#ifndef WEBUI_H
#define WEBUI_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

class Sensors;
class PlantCatalog;
class PolivManager;
class TurretManager;
class Notifier;

/**
 * @brief Класс WebUI отвечает за маршруты веб-интерфейса:
 * - Статические файлы из LittleFS (index.html, catalog.html и т.д.)
 * - Динамические эндпоинты (например, /status для получения JSON с данными)
 * - В будущем: /search для поиска растений, /control для управления поливом.
 *
 * При инициализации (begin) мы передаём ссылки на модули (sensors, plantCatalog и др.)
 * и ссылку на уже созданный в main.cpp AsyncWebServer.
 * 
 * Обновления (update) — пока не нужны, но могут пригодиться для периодических задач.
 */
class WebUI {
public:
    WebUI();
    bool begin(AsyncWebServer* srv, Sensors* sensors, PlantCatalog* plantCatalog, PolivManager* polivManager, TurretManager* turretManager, Notifier* notifier);
    void update();

private:
    AsyncWebServer* server;
    Sensors* sensors;
    PlantCatalog* plantCatalog;
    PolivManager* polivManager;
    TurretManager* turretManager;
    Notifier* notifier;

    void handleRoot(AsyncWebServerRequest *request);
    void handleStatus(AsyncWebServerRequest *request);

    // В будущем:
    // void handleSearch(AsyncWebServerRequest *request);
    // void handlePlantSelect(AsyncWebServerRequest *request);
    // и т.д.
};

#endif // WEBUI_H
