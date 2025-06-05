// Configurações para o dispositivo da Sala de Aula 1
#define WIFI_SSID "SuaRedeWiFi"
#define WIFI_PASSWORD "SuaSenhaWiFi"
#define DEVICE_IP IPAddress(192, 168, 1, 114)
#define DEVICE_GATEWAY IPAddress(192, 168, 1, 1)
#define DEVICE_SUBNET IPAddress(255, 255, 255, 0)
#define DJANGO_SERVER_IP "192.168.1.100"
#define DJANGO_SERVER_PORT 8000
#define DEVICE_TAG "ar-sala-aula-1"
// Definições de pinos para sala_aula_1 (se diferentes, caso contrário pode usar os de firmware_base ou definir aqui)
#define PINO_IR_TX 23 // Exemplo, ajuste se necessário
#define PINO_LED_STATUS 2 // Exemplo, ajuste se necessário

// Incluir temperature_codes.h que está junto com o firmware_base
// Nota: Se os códigos IR forem diferentes, este include precisaria ser específico do dispositivo
#include "../firmware_base/src/temperature_codes.h"
