#include <Arduino.h>
#include "config.h"
#include "WiFiManager.h"
#include "Sensors.h"
#include "PlantCatalog.h"
#include "WebUI.h"
#include "PolivManager.h"
#include "TurretManager.h"
#include "Notifier.h"
#include <ESPmDNS.h>
#include <FS.h>
#include <LittleFS.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

// Модули
WiFiManager wifiManager;
Sensors sensors;
PlantCatalog plantCatalog;
PolivManager polivManager;
TurretManager turretManager;
Notifier notifier;
AsyncWebServer server(80);

// DNS для AP
DNSServer dnsServer;
IPAddress apIP(192,168,4,1);

void startCaptivePortal();
void setupCaptivePortalRoutes();
void startNormalMode();

const char* adminUser = "admin";
const char* adminPass = "admin";

// Проверка авторизации
bool isAuthenticated(AsyncWebServerRequest *request) {
  if (request->hasHeader("Cookie")) {
    String cookie = request->header("Cookie");
    if (cookie.indexOf("session=1") != -1) {
      return true;
    }
  }
  return false;
}

// Если не авторизован - редирект
void handleNotAuthenticated(AsyncWebServerRequest *request) {
  request->redirect("/login.html");
}

String generateLoginPage(const String& errorMsg="") {
  String errorHTML="";
  if (!errorMsg.isEmpty()) {
    errorHTML = "<div class='error-message' style='display:block;color:red;margin-top:1em;'>" + errorMsg + "</div>";
  }

  return String(
    "<!DOCTYPE html><html lang='ru'><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<title>Авторизация</title>"
    "<link rel='stylesheet' href='style.css'>"
    "</head><body class='light-theme'>"
    "<div class='page-container'>"
    "<div class='content-panel'>"
    "<h1 class='setup-title'>Авторизация</h1>"
    "<p>Логин и пароль чувствительны к регистру.</p>"
    "<form method='POST' action='/login' class='wifi-form' style='width:100%;'>"
    "<input type='text' name='username' class='input-field' placeholder='Логин' required>"
    "<input type='password' name='password' class='input-field' placeholder='Пароль' required>"
    "<input type='submit' value='Ввод' class='menu-btn'>"
    "</form>"
    + errorHTML +
    "</div></div></body></html>"
  );
}

String generateWifiSetupPage(const String& errorMsg="") {
  String errorHTML="";
  if (!errorMsg.isEmpty()) {
    errorHTML="<div class='error-message' style='display:block;margin-top:1em;'>"+errorMsg+"</div>";
  } else {
    errorHTML="<div class='error-message' style='display:none;'></div>";
  }

  return String(
    "<!DOCTYPE html><html lang='ru'><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<title>Настройка Wi-Fi</title>"
    "<link rel='stylesheet' href='style.css'>"
    "</head><body class='light-theme'>"
    "<div class='page-container'>"
    "<div class='content-panel'>"
    "<h1 class='setup-title'>Настройка Wi-Fi</h1>"
    "<p>Логин и пароль Wi-Fi чувствительны к регистру.</p>"
    "<form method='POST' action='/save_wifi' class='wifi-form' style='width:100%;'>"
    "<input type='text' name='ssid' class='input-field' placeholder='Имя сети (SSID)' required maxlength='32'>"
    "<input type='password' name='pass' class='input-field' placeholder='Пароль' required maxlength='64'>"
    "<input type='submit' value='Сохранить' class='menu-btn'>"
    "</form>"
    + errorHTML +
    "</div></div></body></html>"
  );
}

String generateSuccessPage() {
  return String(
    "<!DOCTYPE html><html lang='ru'><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<title>Успех</title>"
    "<link rel='stylesheet' href='style.css'>"
    "</head><body class='light-theme'>"
    "<div class='page-container'>"
    "<div class='content-panel'>"
    "<div class='icon-text'>"
    "<img src='/images/icons/success_icon.png' alt='Успех' class='status-icon'>"
    "<span class='success-message'>Данные успешно сохранены!</span>"
    "</div>"
    "<p>Устройство подключается к вашей сети...<br>"
    "Через несколько секунд вы сможете перейти на <strong>http://myirrigationsystem.local/</strong> для доступа к главной странице.</p>"
    "</div></div></body></html>"
  );
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("System startup...");

  if(!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed!");
  } else {
    Serial.println("LittleFS mount success!");
  }

  wifiManager.begin();

  if (wifiManager.tryConnectSaved()) {
    Serial.println("Connected to saved WiFi. Normal mode...");
    startNormalMode();
  } else {
    Serial.println("No saved WiFi. Starting Captive Portal...");
    startCaptivePortal();
  }
}

void loop() {
  dnsServer.processNextRequest();
  if (wifiManager.isConnected()) {
    sensors.update();
    polivManager.update();
    turretManager.update();
    notifier.update();
  }
}

void startCaptivePortal() {
  wifiManager.startAPMode("IrrigationSystem_Setup");
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", apIP);

  setupCaptivePortalRoutes();
  server.begin();

  Serial.println("Connect to 'IrrigationSystem_Setup', then open http://myirrigationsystem.local/ or scan QR.");
  // Чтобы Android не отображал Captive Portal, вернём 204 на некоторые известные адреса
}

void setupCaptivePortalRoutes() {
  // Стандартные адреса для гашения Captive Portal
  server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(204, "text/plain", "");
  });
  server.on("/connectivitycheck.gstatic.com/generate_204", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(204, "text/plain", "");
  });

  server.on("/login.html", HTTP_GET, [](AsyncWebServerRequest *request){
    if(isAuthenticated(request)) {
      request->redirect("/wifi_setup.html");
      return;
    }
    request->send(200, "text/html; charset=UTF-8", generateLoginPage());
  });

  server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request){
    String username, password;
    if(request->hasArg("username")) username = request->arg("username");
    if(request->hasArg("password")) password = request->arg("password");

    if(username == adminUser && password == adminPass) {
      AsyncWebServerResponse *response = request->beginResponse(200, "text/html", "<meta http-equiv='refresh' content='0; url=/wifi_setup.html'>");
      response->addHeader("Set-Cookie", "session=1");
      request->send(response);
    } else {
      request->send(200, "text/html; charset=UTF-8", generateLoginPage("Логин или пароль введены неправильно"));
    }
  });

  server.on("/wifi_setup.html", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!isAuthenticated(request)) {
      handleNotAuthenticated(request);
      return;
    }
    request->send(200, "text/html; charset=UTF-8", generateWifiSetupPage());
  });

  server.on("/save_wifi", HTTP_POST, [](AsyncWebServerRequest *request){
    if(!isAuthenticated(request)) {
      handleNotAuthenticated(request);
      return;
    }

    String ssid, pass;
    if(request->hasArg("ssid")) ssid = request->arg("ssid");
    if(request->hasArg("pass")) pass = request->arg("pass");

    if(ssid.isEmpty() || ssid.length()>32 || pass.isEmpty() || pass.length()>64) {
      request->send(200, "text/html; charset=UTF-8", generateWifiSetupPage("Неверные данные. Попробуйте снова."));
      return;
    }

    wifiManager.saveCredentials(ssid.c_str(), pass.c_str());

    WiFi.begin(ssid.c_str(), pass.c_str());
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(200);
    }

    if (WiFi.status() == WL_CONNECTED) {
      // Успех
      request->send(200, "text/html; charset=UTF-8", generateSuccessPage());
      delay(5000);
      ESP.restart();
    } else {
      // Не удалось подключиться, остаёмся в AP-режиме
      request->send(200, "text/html; charset=UTF-8", generateWifiSetupPage("Не удалось подключиться. Проверьте данные."));
    }
  });

  server.serveStatic("/style.css", LittleFS, "/style.css");
  server.serveStatic("/images/", LittleFS, "/images/");
  
  server.onNotFound([](AsyncWebServerRequest *request){
    // Если не авторизован, редирект на login
    if(!isAuthenticated(request) && request->url() != "/login.html") {
      request->redirect("/login.html");
      return;
    }
    request->send(404, "text/plain", "Not Found");
  });
}

void startNormalMode() {
  if (MDNS.begin("myirrigationsystem")) {
    Serial.println("mDNS: http://myirrigationsystem.local/");
  }

  if (!sensors.begin()) Serial.println("Sensors init failed!");
  else Serial.println("Sensors OK!");

  if (!plantCatalog.begin()) Serial.println("PlantCatalog init failed!");
  else Serial.println("PlantCatalog loaded.");

  if (!turretManager.begin()) Serial.println("TurretManager init failed.");
  else Serial.println("TurretManager OK!");

  if (!polivManager.begin(&sensors, &plantCatalog)) Serial.println("PolivManager init failed!");
  else Serial.println("PolivManager OK!");

  if (!notifier.begin()) Serial.println("Notifier init failed!");
  else Serial.println("Notifier OK!");

  static WebUI webUI;
  if (!webUI.begin(&server, &sensors, &plantCatalog, &polivManager, &turretManager, &notifier)) {
    Serial.println("WebUI init failed!");
  } else {
    Serial.println("Web interface at http://myirrigationsystem.local/ or device IP");
  }

  server.begin();
  Serial.println("Normal mode started.");
}
