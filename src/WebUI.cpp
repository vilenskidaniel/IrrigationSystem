// WebUI.cpp
#include "WebUI.h"
#include "Sensors.h"
#include "PlantCatalog.h"
#include "PolivManager.h"
#include "TurretManager.h"
#include "Notifier.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

WebUI::WebUI() : server(nullptr), sensors(nullptr), plantCatalog(nullptr), polivManager(nullptr), turretManager(nullptr), notifier(nullptr)
{
}

bool WebUI::begin(AsyncWebServer* srv, Sensors* s, PlantCatalog* pc, PolivManager* pm, TurretManager* tm, Notifier* n) {
    server = srv;
    sensors = s;
    plantCatalog = pc;
    polivManager = pm;
    turretManager = tm;
    notifier = n;

    if (!server || !sensors || !plantCatalog || !polivManager || !turretManager || !notifier) {
        Serial.println("WebUI: One or more dependencies are null!");
        return false;
    }

    // Статические файлы
    // Главная страница index.html
    server->serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    // Другие страницы:
    // Каталог
    server->serveStatic("/catalog.html", LittleFS, "/catalog.html");
    // Система полива
    server->serveStatic("/watering.html", LittleFS, "/watering.html");
    // Видеотека
    server->serveStatic("/video.html", LittleFS, "/video.html");
    // Турель
    server->serveStatic("/turret.html", LittleFS, "/turret.html");
    // Настройки
    server->serveStatic("/settings.html", LittleFS, "/settings.html");
    // Wi-Fi Setup (для captive portal)
    server->serveStatic("/wifi_setup.html", LittleFS, "/wifi_setup.html");

    // Маршрут для главной
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleRoot(request);
    });

    // Маршрут для получения статуса в JSON
    server->on("/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleStatus(request);
    });

    // В будущем можно добавить другие маршруты: /search, /control и т.д.

    Serial.println("WebUI: HTTP routes configured.");
    return true;
}

void WebUI::update() {
    // Пока ничего не делаем.
}

void WebUI::handleRoot(AsyncWebServerRequest *request) {
    // Если есть index.html, он будет отдан statically.
    // Если хотим добавить логику fallback:
    if (!LittleFS.exists("/index.html")) {
        request->send(200, "text/plain", "Main page not found!");
        return;
    }

    // Если не хотим полагаться на serveStatic defaultFile, можно вручную:
    request->send(LittleFS, "/index.html", String(), false);
}

void WebUI::handleStatus(AsyncWebServerRequest *request) {
    // Возвращаем текущее состояние датчиков в JSON.
    DynamicJsonDocument doc(1024);
    doc["temperature"] = sensors->getTemperature();
    doc["humidity"] = sensors->getHumidity();
    doc["lux"] = sensors->getLux();

    JsonArray soilArr = doc.createNestedArray("soilMoisture");
    for (int i=0; i<SOIL_SENSOR_COUNT; i++) {
        soilArr.add(sensors->getSoilMoisture(i));
    }

    // Пример: добавим статус турели и полива
    doc["turretAvailable"] = turretManager->isAvailable();

    // Можно также добавить информацию о состоянии слотов полива
    // Но пока опустим, или будем делать AJAX вызов в будущем.

    String response;
    serializeJson(doc, response);

    request->send(200, "application/json", response);
}
