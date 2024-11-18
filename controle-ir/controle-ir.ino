#include <Arduino.h>
#include <IRremote.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <stdlib.h> // Biblioteca para alocação dinâmica de memória

// Pino do LED IR
const int PINO_IR = 23; // Ajuste conforme sua conexão

// Definição dos pinos dos botões
const int PINO_BOTAO_LIGAR = 14;        // Pino para o botão "Ligar"
const int PINO_BOTAO_DESLIGAR = 25;     // Pino para o botão "Desligar"
const int PINO_BOTAO_AUMENTAR_TEMP = 33; // Pino para o botão "Aumentar Temperatura"
const int PINO_BOTAO_DIMINUIR_TEMP = 32; // Pino para o botão "Diminuir Temperatura"

#include "temperature_codes.h" // Inclui os dados de temperatura

// Variáveis de conexão Wi-Fi
const char* ssid = "Fabnet";      // Substitua pelo seu SSID
const char* senha = "71037523";   // Substitua pela sua senha

AsyncWebServer servidor(80);

uint8_t temperaturaAtual = 20; // Temperatura inicial definida para 20°C
bool arCondicionadoLigado = false; // Estado inicial do ar-condicionado

// Variáveis para debounce dos botões
const unsigned long atrasoDebounce = 50; // Tempo de debounce de 50ms

struct Botao {
  const int pino;
  unsigned long ultimoTempoDebounce;
  bool estadoAnterior;
};

Botao botaoLigar = {PINO_BOTAO_LIGAR, 0, HIGH};
Botao botaoDesligar = {PINO_BOTAO_DESLIGAR, 0, HIGH};
Botao botaoAumentar = {PINO_BOTAO_AUMENTAR_TEMP, 0, HIGH};
Botao botaoDiminuir = {PINO_BOTAO_DIMINUIR_TEMP, 0, HIGH};

void setup() {
  Serial.begin(115200);
  Serial.println("Inicializando setup...");
  IrSender.begin(PINO_IR);
  Serial.println("Emissor IR iniciado");

  // Configuração dos pinos dos botões como entrada com pull-up interno
  pinMode(PINO_BOTAO_LIGAR, INPUT_PULLUP);
  pinMode(PINO_BOTAO_DESLIGAR, INPUT_PULLUP);
  pinMode(PINO_BOTAO_AUMENTAR_TEMP, INPUT_PULLUP);
  pinMode(PINO_BOTAO_DIMINUIR_TEMP, INPUT_PULLUP);
  Serial.println("Pinos dos botões configurados");

  // Conexão à rede Wi-Fi
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

  // Configuração das rotas HTTP
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
    if (arCondicionadoLigado) {
      status += "Ligado";
    } else {
      status += "Desligado";
    }
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

  // Inicia o servidor
  servidor.begin();
  Serial.println("Servidor iniciado");
}

void loop() {
  verificarBotao(botaoLigar, enviarComandoLigar, "Ligar");
  verificarBotao(botaoDesligar, enviarComandoDesligar, "Desligar");
  verificarBotao(botaoAumentar, aumentarTemperatura, "Aumentar Temperatura");
  verificarBotao(botaoDiminuir, diminuirTemperatura, "Diminuir Temperatura");
}

void verificarBotao(Botao& botao, void (*acao)(), const char* nomeBotao) {
  bool leituraAtual = digitalRead(botao.pino);
  unsigned long tempoAtual = millis();

  if (leituraAtual != botao.estadoAnterior) {
    botao.ultimoTempoDebounce = tempoAtual;
    botao.estadoAnterior = leituraAtual;
  }

  if ((tempoAtual - botao.ultimoTempoDebounce) > atrasoDebounce) {
    if (leituraAtual == LOW) {
      Serial.print("Botão '");
      Serial.print(nomeBotao);
      Serial.println("' pressionado");
      acao();
      // Aguarda até que o botão seja liberado para evitar múltiplas detecções
      while (digitalRead(botao.pino) == LOW) {
        delay(10); // Pequeno delay para evitar bloquear completamente o loop
      }
    }
  }
}

bool enviarComandoTemperatura(uint8_t temperatura) {
  if (!arCondicionadoLigado) {
    Serial.println("Ar-condicionado está desligado. Ligue-o primeiro.");
    return false;
  }
  Serial.print("Enviando comando de temperatura: ");
  Serial.println(temperatura);
  // Encontre o código correspondente à temperatura desejada
  for (int i = 0; i < sizeof(temperatureCodes) / sizeof(temperatureCodes[0]); i++) {
    if (temperatureCodes[i].temperature == temperatura) {
      // Aloca dinamicamente um buffer para armazenar os dados
      uint16_t* buffer = (uint16_t*)malloc(temperatureCodes[i].length * sizeof(uint16_t));
      if (buffer == nullptr) {
        Serial.println("Erro ao alocar memória para o buffer");
        return false;
      }
      // Copie os dados da flash para o buffer
      memcpy_P(buffer, temperatureCodes[i].rawData, temperatureCodes[i].length * sizeof(uint16_t));
      // Envie os dados
      IrSender.sendRaw(buffer, temperatureCodes[i].length, 38);
      // Libera a memória alocada
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

void enviarComandoLigar() {
  Serial.println("Enviando comando 'Ligar'");
  temperaturaAtual = 20;
  arCondicionadoLigado = true; // Atualiza o estado para ligado

  // Aloca dinamicamente um buffer para armazenar os dados
  uint16_t* buffer = (uint16_t*)malloc(onCommand20.length * sizeof(uint16_t));
  if (buffer == nullptr) {
    Serial.println("Erro ao alocar memória para o buffer");
    return;
  }

  // Copie os dados da flash para o buffer
  memcpy_P(buffer, onCommand20.rawData, onCommand20.length * sizeof(uint16_t));

  // Envie os dados
  IrSender.sendRaw(buffer, onCommand20.length, 38);

  // Libera a memória alocada
  free(buffer);

  Serial.println("Comando 'Ligar' enviado com temperatura de 20°C.");
}

void enviarComandoDesligar() {
  Serial.println("Enviando comando 'Desligar'");
  arCondicionadoLigado = false; // Atualiza o estado para desligado

  // Aloca dinamicamente um buffer para armazenar os dados
  uint16_t* buffer = (uint16_t*)malloc(offCommand.length * sizeof(uint16_t));
  if (buffer == nullptr) {
    Serial.println("Erro ao alocar memória para o buffer");
    return;
  }

  // Copie os dados da flash para o buffer
  memcpy_P(buffer, offCommand.rawData, offCommand.length * sizeof(uint16_t));
  // Envie os dados
  IrSender.sendRaw(buffer, offCommand.length, 38);

  // Libera a memória alocada
  free(buffer);

  Serial.println("Comando 'Desligar' enviado");
}
