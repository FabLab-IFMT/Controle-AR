// Configurações para o dispositivo da Sala de Reuniões
#define WIFI_SSID "Fabnet"
#define WIFI_PASSWORD "71037523"
#define DEVICE_IP IPAddress(192, 168, 1, 113)
#define DEVICE_GATEWAY IPAddress(192, 168, 1, 1)
#define DEVICE_SUBNET IPAddress(255, 255, 255, 0)
#define DJANGO_SERVER_IP "192.168.1.100"
#define DJANGO_SERVER_PORT 8000
#define DEVICE_TAG "ar-sala-reunioes"
#define PINO_IR_TX 23
#define PINO_LED_STATUS 2
#define PINO_SENSOR_TEMPERATURA 36
#define PINO_SENSOR_CORRENTE 39

// Incluir temperature_codes.h que está junto com o firmware_base
#include "../firmware_base/src/temperature_codes.h"
