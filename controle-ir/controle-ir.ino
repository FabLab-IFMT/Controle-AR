#include <Arduino.h>
#include <IRremote.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <stdlib.h> // Para alocação dinâmica de memória

#include "temperature_codes.h" // Arquivo que contém os códigos de frequência/temperatura

// Pino do LED IR
const int PINO_IR = 23; // Ajuste conforme sua conexão

// Variáveis de conexão Wi-Fi
const char* ssid = "Fabnet";      // Substitua pelo seu SSID
const char* senha = "71037523";   // Substitua pela sua senha

AsyncWebServer servidor(80);

uint8_t temperaturaAtual = 20; // Temperatura inicial definida para 20°C
bool arCondicionadoLigado = false; // Estado inicial do ar-condicionado

// Não há mais definições de pinos e debounce para botões

// Função setup: inicializa o IR, conecta no Wi-Fi e configura os endpoints HTTP
void setup() {
  Serial.begin(115200);
  Serial.println("Inicializando setup...");

  // Inicializa o emissor IR
  IrSender.begin(PINO_IR);
  Serial.println("Emissor IR iniciado");

  // Conecta na rede Wi-Fi
  WiFi.begin(ssid, senha);
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(ssid);

  unsigned long inicioTentativa = millis();
  const unsigned long tempoLimiteWiFi = 30000; // Timeout de 30 segundos

  while (WiFi.status() != WL_CONNECTED && millis() - inicioTentativa < tempoLimiteWiFi) {
    delay(1000);
    Serial.println("Conectando ao Wi-Fi...");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Conectado ao Wi-Fi com o endereço IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Falha ao conectar ao Wi-Fi. Tempo limite alcançado.");
  }

  // Configuração dos endpoints HTTP

  servidor.on("/ligar", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Recebida requisição: /ligar");
    enviarComandoLigar();
    request->send(200, "text/plain", "Ar-condicionado ligado.");
  });

  servidor.on("/desligar", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Recebida requisição: /desligar");
    enviarComandoDesligar();
    request->send(200, "text/plain", "Ar-condicionado desligado.");
  });

  servidor.on("/aumentar", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Recebida requisição: /aumentar");
    aumentarTemperatura();
    request->send(200, "text/plain", "Temperatura aumentada.");
  });

  servidor.on("/diminuir", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Recebida requisição: /diminuir");
    diminuirTemperatura();
    request->send(200, "text/plain", "Temperatura diminuída.");
  });

  servidor.on("/definir_temperatura", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Recebida requisição: /definir_temperatura");
    if (request->hasParam("temp")) {
      String parametroTemp = request->getParam("temp")->value();
      int temp = parametroTemp.toInt();
      Serial.print("Parâmetro de temperatura recebido: ");
      Serial.println(temp);
      if (temp >= MIN_TEMPERATURE && temp <= MAX_TEMPERATURE) {
        temperaturaAtual = temp;
        if (enviarComandoTemperatura(temperaturaAtual)) {
          String resposta = "Temperatura definida para " + String(temperaturaAtual) + "°C.";
          request->send(200, "text/plain", resposta);
        } else {
          request->send(400, "text/plain", "Ar-condicionado está desligado. Ligue-o primeiro.");
        }
      } else {
        Serial.println("Temperatura fora do intervalo");
        request->send(400, "text/plain", "Temperatura fora do intervalo.");
      }
    } else {
      Serial.println("Parâmetro 'temp' não fornecido");
      request->send(400, "text/plain", "Parâmetro 'temp' não fornecido.");
    }
  });

  servidor.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Recebida requisição: /status");
    String status = "Temperatura atual: " + String(temperaturaAtual) + "°C.\n";
    status += "Estado: ";
    status += (arCondicionadoLigado ? "Ligado" : "Desligado");
    request->send(200, "text/plain", status);
  });

  servidor.on("/temperatura", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Recebida requisição: /temperatura");
    String resposta = "Temperatura atual: " + String(temperaturaAtual) + "°C.\n";
    if (temperaturaAtual == MAX_TEMPERATURE) {
      resposta += "Temperatura máxima atingida.";
    }
    request->send(200, "text/plain", resposta);
  });

  // Inicia o servidor HTTP
  servidor.begin();
  Serial.println("Servidor iniciado");
}

void loop() {
  // Loop vazio, pois o ESPAsyncWebServer lida com as requisições de forma assíncrona.
}

// Função para enviar comando de temperatura
bool enviarComandoTemperatura(uint8_t temperatura) {
  if (!arCondicionadoLigado) {
    Serial.println("Ar-condicionado está desligado. Ligue-o primeiro.");
    return false;
  }
  Serial.print("Enviando comando de temperatura: ");
  Serial.println(temperatura);
  for (int i = 0; i < sizeof(temperatureCodes) / sizeof(temperatureCodes[0]); i++) {
    if (temperatureCodes[i].temperature == temperatura) {
      uint16_t* buffer = (uint16_t*)malloc(temperatureCodes[i].length * sizeof(uint16_t));
      if (buffer == nullptr) {
        Serial.println("Erro ao alocar memória para o buffer");
        return false;
      }
      memcpy_P(buffer, temperatureCodes[i].rawData, temperatureCodes[i].length * sizeof(uint16_t));
      IrSender.sendRaw(buffer, temperatureCodes[i].length, 38);
      free(buffer);
      Serial.print("Comando de temperatura enviado: ");
      Serial.print(temperatura);
      Serial.println("°C");
      return true;
    }
  }
  Serial.println("Temperatura não encontrada nos códigos armazenados.");
  return false;
}

// Função para aumentar a temperatura
void aumentarTemperatura() {
  Serial.println("Aumentando temperatura");
  if (temperaturaAtual < MAX_TEMPERATURE) {
    temperaturaAtual++;
    if (!enviarComandoTemperatura(temperaturaAtual)) {
      temperaturaAtual--; // Reverte se não conseguiu enviar
    }
  } else {
    Serial.println("Temperatura máxima alcançada.");
  }
}

// Função para diminuir a temperatura
void diminuirTemperatura() {
  Serial.println("Diminuindo temperatura");
  if (temperaturaAtual > MIN_TEMPERATURE) {
    temperaturaAtual--;
    if (!enviarComandoTemperatura(temperaturaAtual)) {
      temperaturaAtual++; // Reverte se não conseguiu enviar
    }
  } else {
    Serial.println("Temperatura mínima alcançada.");
  }
}

// Função para ligar o ar-condicionado
void enviarComandoLigar() {
  Serial.println("Enviando comando 'Ligar'");
  temperaturaAtual = 20;
  arCondicionadoLigado = true;

  uint16_t* buffer = (uint16_t*)malloc(onCommand20.length * sizeof(uint16_t));
  if (buffer == nullptr) {
    Serial.println("Erro ao alocar memória para o buffer");
    return;
  }
  memcpy_P(buffer, onCommand20.rawData, onCommand20.length * sizeof(uint16_t));
  IrSender.sendRaw(buffer, onCommand20.length, 38);
  free(buffer);
  Serial.println("Comando 'Ligar' enviado com temperatura de 20°C.");
}

// Função para desligar o ar-condicionado
void enviarComandoDesligar() {
  Serial.println("Enviando comando 'Desligar'");
  arCondicionadoLigado = false;

  uint16_t* buffer = (uint16_t*)malloc(offCommand.length * sizeof(uint16_t));
  if (buffer == nullptr) {
    Serial.println("Erro ao alocar memória para o buffer");
    return;
  }
  memcpy_P(buffer, offCommand.rawData, offCommand.length * sizeof(uint16_t));
  IrSender.sendRaw(buffer, offCommand.length, 38);
  free(buffer);
  Serial.println("Comando 'Desligar' enviado");
}
