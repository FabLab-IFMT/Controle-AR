// firmware_base/src/wifi_manager.cpp
#include "wifi_manager.h"
#include "config.h" // Para WIFI_SSID, WIFI_PASSWORD, PINO_LED_STATUS, etc.

extern bool wifiConectado; // Definida em firmware_base.ino, idealmente seria gerenciada dentro deste módulo
// extern int PINO_LED_STATUS; // Exemplo, se PINO_LED_STATUS for usado aqui. Removido pois PINO_LED_STATUS é macro de config.h

void setupWiFi() {
  // TODO: Mover lógica de WiFi.mode, WiFi.config, WiFi.begin daqui do firmware_base.ino
  // Exemplo:
  // WiFi.mode(WIFI_STA);
  // if (strlen(DEVICE_IP.toString().c_str()) > 0 && DEVICE_IP[0] != 0) { // Verifica se IP estático está configurado
  //   if (!WiFi.config(DEVICE_IP, DEVICE_GATEWAY, DEVICE_SUBNET)) {
  //     Serial.println("Falha na configuração de IP fixo");
  //   }
  // }
  // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // Serial.print("Conectando ao Wi-Fi: "); Serial.println(WIFI_SSID);
  // unsigned long inicioTentativa = millis();
  // const unsigned long tempoLimiteWiFi = 30000;
  // while (WiFi.status() != WL_CONNECTED && millis() - inicioTentativa < tempoLimiteWiFi) {
  //   digitalWrite(PINO_LED_STATUS, !digitalRead(PINO_LED_STATUS));
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println();
  // if (WiFi.status() == WL_CONNECTED) {
  //   wifiConectado = true;
  //   Serial.print("Conectado ao Wi-Fi com o endereço IP: "); Serial.println(WiFi.localIP());
  //   digitalWrite(PINO_LED_STATUS, HIGH);
  // } else {
  //   wifiConectado = false;
  //   Serial.println("Falha ao conectar ao Wi-Fi. Tempo limite alcançado.");
  //   digitalWrite(PINO_LED_STATUS, LOW);
  // }
}

void handleWiFiConnection() {
  // TODO: Mover lógica de verificação de WiFi.status() e reconexão daqui do firmware_base.ino
  // Exemplo:
  // if (WiFi.status() != WL_CONNECTED) {
  //   if (wifiConectado) { // Se perdeu a conexão mas estava conectado antes
  //     Serial.println("Conexão WiFi perdida. Tentando reconectar...");
  //     wifiConectado = false;
  //     digitalWrite(PINO_LED_STATUS, LOW);
  //     WiFi.reconnect(); // Ou WiFi.begin() novamente
  //   }
  // } else {
  //   if (!wifiConectado) { // Se reconectou
  //     Serial.print("WiFi reconectado. IP: "); Serial.println(WiFi.localIP());
  //     wifiConectado = true;
  //     digitalWrite(PINO_LED_STATUS, HIGH);
  //   }
  // }
}

bool isWifiConnected() {
  return wifiConectado; // Ou return (WiFi.status() == WL_CONNECTED); para verificação em tempo real
}
