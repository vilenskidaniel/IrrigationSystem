#ifndef NOTIFIER_H
#define NOTIFIER_H

#include <Arduino.h>

/**
 * @brief Класс для отправки уведомлений, в данном случае — через Telegram.
 * 
 * Этот класс берёт токен и chat_id из config.h (TELEGRAM_TOKEN, TELEGRAM_CHAT_ID).
 * При необходимости отправить уведомление, он формирует HTTP(S) GET-запрос к Telegram API.
 * 
 * В будущем можно расширить функционал:
 * - Добавить разные типы уведомлений (текст, фото и т.д.)
 * - Добавить очередь сообщений и асинхронную отправку.
 * 
 * Безопасность: хранение токена в config.h не безопасно для публичного кода.
 * Рекомендуется вынести в отдельное приватное хранилище или получить в runtime.
 */
class Notifier {
public:
    Notifier();

    // Инициализация модуля (пока просто флаг включения/выключения)
    bool begin(); 

    // Обновление — пока пусто, но в будущем можно реализовать логику очередей
    void update();

    // Включить/выключить уведомления
    void setEnabled(bool en);

    bool isEnabled() const;

    // Отправить произвольное сообщение
    bool notify(const String& message);

    // Специальные уведомления
    bool notifyPolivError(int slotIndex);
    bool notifyCriticalDryness(int slotIndex);
    bool notifyTurretAlert(const String& what);
    bool notifySystemMessage(const String& msg);

private:
    bool enabled;
    bool sendTelegramMessage(const String& msg);

    String urlEncode(const String& str);
};

#endif // NOTIFIER_H
