// Firmware para Sala de Aula 1
#include "config.h" // Configurações específicas deste dispositivo

// Assumindo que agora queremos que ele use o firmware_base completo:
// (Veja comentários em sala_reunioes_device.ino sobre a inclusão e compilação)

void setup() {
  // O setup() principal está em firmware_base.ino
  // Ele usará as configurações de config.h
}

void loop() {
  // O loop() principal está em firmware_base.ino
}

// Comentando o código original que era específico e simples para este dispositivo.
/*
Original sala_aula_1_device.ino content:
#include <WiFi.h>
#include <HTTPClient.h>

// ============= CONFIGURAÇÃO DE REDE =============
// Variáveis de conexão Wi-Fi
const char* ssid = "SuaRedeWiFi";      // Substitua pelo seu SSID
const char* senha = "SuaSenhaWiFi";    // Substitua pela sua senha

// Configuração de IP fixo (opcional)
IPAddress ip(192, 168, 1, 114);        // IP diferente para o segundo ESP32
IPAddress gateway(192, 168, 1, 1);     // Gateway da sua rede
IPAddress subnet(255, 255, 255, 0);    // Máscara de sub-rede

// Endereço do servidor Django
const char* servidorDjango = "192.168.1.100";  // Substitua pelo IP do seu servidor Django
const int portaDjango = 8000;                  // Porta do servidor Django
const char* tagDispositivo = "ar-sala-aula-1"; // Identificador correspondente ao cadastrado no Django

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, senha);

  // Configuração de IP fixo
  if (!WiFi.config(ip, gateway, subnet)) {
    Serial.println("Falha na configuração de IP fixo");
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }

  Serial.println("Conectado ao WiFi");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String("http://") + servidorDjango + ":" + String(portaDjango) + "/api/controle-ir/" + tagDispositivo + "/";
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.println("Erro na requisição HTTP");
    }

    http.end();
  } else {
    Serial.println("WiFi desconectado");
  }

  delay(10000); // Aguarda 10 segundos antes de fazer nova requisição
}
*/
