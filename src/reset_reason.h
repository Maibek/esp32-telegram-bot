#pragma once
#include <esp_system.h>

String lastResetReason = "Неизвестно";

String getResetReason() {
    esp_reset_reason_t reason = esp_reset_reason();
    switch (reason) {
        case ESP_RST_POWERON:      return "Power-on / включение питания";
        case ESP_RST_EXT:          return "Внешний reset (кнопка EN)";
        case ESP_RST_SW:           return "Software reset (ESP.restart())";
        case ESP_RST_PANIC:        return "Panic / Guru Meditation Error";
        case ESP_RST_INT_WDT:      return "Interrupt Watchdog Timeout";
        case ESP_RST_TASK_WDT:     return "Task Watchdog Timeout";
        case ESP_RST_WDT:          return "Other Watchdog Timeout";
        case ESP_RST_DEEPSLEEP:    return "Wake from Deep Sleep";
        case ESP_RST_BROWNOUT:     return "Brownout reset (низкое напряжение)";
        case ESP_RST_SDIO:         return "SDIO reset";
        default:                   return "Неизвестная причина (" + String(reason) + ")";
    }
}

void printResetReason() {
    Serial.printf("Причина перезагрузки: %s\n", getResetReason().c_str());
}