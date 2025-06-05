// firmware_base/src/wifi_manager.h
#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

void setupWiFi();
void handleWiFiConnection();
bool isWifiConnected();

#endif
