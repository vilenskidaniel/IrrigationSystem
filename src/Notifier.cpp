#include "Notifier.h"
#include "config.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>

Notifier::Notifier() : enabled(TELEGRAM_ENABLED) {
}

bool Notifier::begin() {
    // Если TELEGRAM_ENABLED = false или нет токена/chat_id, отключаем уведомления
    if (!TELEGRAM_ENABLED || strlen(TELEGRAM_TOKEN) == 0 || strlen(TELEGRAM_CHAT_ID) == 0) {
        Serial.println("Notifier: Telegram disabled or no token/chat_id provided");
        enabled = false;
    } else {
        Serial.println("Notifier: Telegram notifications enabled");
    }

    return true; 
}

void Notifier::update() {
    // Пока логика пустая. В будущем можно добавить асинхронную отправку или ретраи.
}

void Notifier::setEnabled(bool en) {
    enabled = en;
}

bool Notifier::isEnabled() const {
    return enabled;
}

bool Notifier::notify(const String& message) {
    if (!enabled) {
        Serial.println("Notifier: notifications disabled");
        return false;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Notifier: No Wi-Fi connection, cannot send notification");
        return false;
    }

    return sendTelegramMessage(message);
}

bool Notifier::notifyPolivError(int slotIndex) {
    String msg = "PolivManager ERROR on slot " + String(slotIndex) + 
                 ". Moisture did not improve within the timeout.";
    return notify(msg);
}

bool Notifier::notifyCriticalDryness(int slotIndex) {
    String msg = "Critical dryness detected on slot " + String(slotIndex) +
                 ". Immediate attention required!";
    return notify(msg);
}

bool Notifier::notifyTurretAlert(const String& what) {
    String msg = "Turret alert: " + what;
    return notify(msg);
}

bool Notifier::notifySystemMessage(const String& msg) {
    return notify("System Message: " + msg);
}

bool Notifier::sendTelegramMessage(const String& msg) {
    // Формируем URL для Telegram API
    String baseURL = "https://api.telegram.org/bot" + String(TELEGRAM_TOKEN) + "/sendMessage";
    String query = "?chat_id=" + String(TELEGRAM_CHAT_ID) + "&text=" + urlEncode(msg);
    String url = baseURL + query;

    WiFiClientSecure client;
    client.setInsecure(); // Без проверки сертификата

    if(!client.connect("api.telegram.org", 443)) {
        Serial.println("Notifier: Failed to connect to api.telegram.org");
        return false;
    }

    // Отправка GET-запроса
    String request = String("GET ") + url + " HTTP/1.1\r\n" +
                     "Host: api.telegram.org\r\n" +
                     "User-Agent: IrrigationSystem/1.0\r\n" +
                     "Connection: close\r\n\r\n";

    client.print(request);

    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println("Notifier: Client Timeout!");
            client.stop();
            return false;
        }
    }

    // Читаем ответ (не обязательно парсить)
    while(client.available()) {
        String line = client.readStringUntil('\n');
        // Можно логировать при необходимости
    }

    Serial.println("Notifier: Telegram message sent.");
    return true;
}

String Notifier::urlEncode(const String& str) {
    // Простая URL-кодировка пробелов и переводов строк.
    String encoded;
    for (int i=0; i<(int)str.length(); i++){
        char c = str.charAt(i);
        if (c == ' ') {
            encoded += "%20";
        } else if (c == '\n') {
            encoded += "%0A";
        } else {
            encoded += c;
        }
    }
    return encoded;
}
