// firmware_base/src/web_server_handler.cpp
#include "web_server_handler.h"
#include "ir_handler.h" // Para chamar funções de controle IR como enviarComandoLigarIR, etc.
#include "config.h"     // Para DEVICE_TAG, DJANGO_SERVER_IP, etc., se necessário em respostas
#include <ArduinoJson.h> // Para qualquer manipulação de JSON, se necessário

// Variáveis globais de estado (declaradas 'extern' pois são definidas no firmware_base.ino)
// Essas são necessárias para responder aos requests /status, /info, etc.
// E para algumas lógicas de comando. Idealmente, um módulo de "estado do dispositivo" gerenciaria isso.
extern uint8_t temperaturaAtual;
extern bool arCondicionadoLigado;
extern String modoOperacao;
extern uint8_t velocidadeVentilador;
extern bool swingAtivado;
extern float temperaturaAmbiente; // Simulada ou lida por um sensor_handler
extern float consumoEnergia;    // Simulado ou calculado

// Funções de controle de estado que interagem com o IR Handler
// Estas funções atualizam o estado E chamam o IR Handler.
// Elas são chamadas pelos endpoints do servidor web.

// Função para aumentar a temperatura (exemplo de como seria)
void aumentarTemperaturaWeb() {
  Serial.println("Web: Aumentando temperatura");
  if (temperaturaAtual < MAX_TEMPERATURE) { // MAX_TEMPERATURE viria de config.h ou temperature_codes.h
    temperaturaAtual++;
    if (!enviarComandoTemperaturaIR(temperaturaAtual)) {
      temperaturaAtual--; // Reverte se não conseguiu enviar
    }
  } else {
    Serial.println("Web: Temperatura máxima alcançada.");
  }
}

// Função para diminuir a temperatura (exemplo)
void diminuirTemperaturaWeb() {
  Serial.println("Web: Diminuindo temperatura");
  if (temperaturaAtual > MIN_TEMPERATURE) { // MIN_TEMPERATURE viria de config.h ou temperature_codes.h
    temperaturaAtual--;
    if (!enviarComandoTemperaturaIR(temperaturaAtual)) {
      temperaturaAtual++; // Reverte se não conseguiu enviar
    }
  } else {
    Serial.println("Web: Temperatura mínima alcançada.");
  }
}

void ligarArCondicionadoWeb() {
    Serial.println("Web: Ligando Ar Condicionado");
    enviarComandoLigarIR(); // Envia comando IR
    arCondicionadoLigado = true; // Atualiza estado
    temperaturaAtual = 20; // Define temperatura padrão ao ligar
    // Nota: enviarComandoLigarIR pode já tentar setar para 20C. Consistência é chave.
}

void desligarArCondicionadoWeb() {
    Serial.println("Web: Desligando Ar Condicionado");
    enviarComandoDesligarIR();
    arCondicionadoLigado = false;
    consumoEnergia = 0.0; // Zera consumo ao desligar
}


void configurarEndpoints() {
  // Rota raiz - retorna informações básicas
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String resposta = "ESP32 Controle AR v1.0 (Modular)\n";
    resposta += "Status: " + String(arCondicionadoLigado ? "Ligado" : "Desligado") + "\n";
    resposta += "Temperatura: " + String(temperaturaAtual) + "°C\n";
    resposta += "Modo: " + modoOperacao + "\n";
    request->send(200, "text/plain", resposta);
  });

  server.on("/ligar", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Web: Recebida requisição: /ligar");
    ligarArCondicionadoWeb();
    request->send(200, "text/plain", "Ar-condicionado ligado.");
  });

  server.on("/desligar", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Web: Recebida requisição: /desligar");
    desligarArCondicionadoWeb();
    request->send(200, "text/plain", "Ar-condicionado desligado.");
  });

  server.on("/aumentar", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Web: Recebida requisição: /aumentar");
    aumentarTemperaturaWeb();
    request->send(200, "text/plain", "Temperatura aumentada.");
  });

  server.on("/diminuir", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Web: Recebida requisição: /diminuir");
    diminuirTemperaturaWeb();
    request->send(200, "text/plain", "Temperatura diminuída.");
  });

  server.on("/definir_temperatura", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Web: Recebida requisição: /definir_temperatura");
    if (request->hasParam("temp")) {
      String parametroTemp = request->getParam("temp")->value();
      int temp = parametroTemp.toInt();
      Serial.print("Web: Parâmetro de temperatura recebido: "); Serial.println(temp);
      if (temp >= MIN_TEMPERATURE && temp <= MAX_TEMPERATURE) {
        if (!arCondicionadoLigado && temp > 0) { // Se tentar definir temp mas está desligado
             // Opcional: ligar automaticamente ou retornar erro.
             // Por agora, vamos permitir definir temp mesmo desligado, mas o IR só envia se ligado.
             // Se quiser ligar: ligarArCondicionadoWeb();
        }
        temperaturaAtual = temp; // Atualiza o estado local
        if (enviarComandoTemperaturaIR(temperaturaAtual)) { // Tenta enviar comando IR
          String resposta = "Temperatura definida para " + String(temperaturaAtual) + "°C.";
          request->send(200, "text/plain", resposta);
        } else {
          // Se enviarComandoTemperaturaIR falhou (ex: AC desligado), o estado local foi atualizado
          // mas o comando IR não. Decida o comportamento:
          // - Enviar erro que AC está desligado.
          // - Enviar sucesso que a temp foi "definida" localmente.
          request->send(200, "text/plain", "Temperatura local definida para " + String(temperaturaAtual) + "°C. AC desligado, comando IR não enviado.");
          // Ou: request->send(400, "text/plain", "Ar-condicionado está desligado. Ligue-o primeiro para enviar comando IR.");
        }
      } else {
        Serial.println("Web: Temperatura fora do intervalo");
        request->send(400, "text/plain", "Temperatura fora do intervalo.");
      }
    } else {
      Serial.println("Web: Parâmetro 'temp' não fornecido");
      request->send(400, "text/plain", "Parâmetro 'temp' não fornecido.");
    }
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Web: Recebida requisição: /status");
    String status = "Temperatura atual: " + String(temperaturaAtual) + "°C.\n";
    status += "Estado: ";
    status += (arCondicionadoLigado ? "Ligado" : "Desligado");
    status += "\nTemperatura ambiente: " + String(temperaturaAmbiente, 1) + "°C.\n";
    status += "Consumo: " + String(consumoEnergia, 2) + " kW/h\n";
    status += "Modo: " + modoOperacao + "\n";
    status += "Velocidade: " + String(velocidadeVentilador) + "\n";
    status += "Swing: " + String(swingAtivado ? "Ativado" : "Desativado") + "\n";
    request->send(200, "text/plain", status);
  });

  // Adicionar outros endpoints de forma similar (/info, /restart, /modo, /velocidade, /swing)
  // Exemplo para /modo:
  server.on("/modo", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("valor")) {
        String novoModo = request->getParam("valor")->value();
        // Validar novoModo aqui
        if (enviarComandoModoIR(novoModo)) {
            modoOperacao = novoModo; // Atualizar estado local
            request->send(200, "text/plain", "Modo alterado para " + novoModo);
        } else {
            request->send(400, "text/plain", "Falha ao alterar modo (AC desligado?)");
        }
    } else {
        request->send(400, "text/plain", "Parâmetro 'valor' não fornecido.");
    }
  });

  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request){
    String info = "ESP32 Controle AR v1.0 (Modular)\n";
    info += "----------------------------------------\n";
    info += "Hardware e Configuração:\n";
    info += "----------------------------------------\n";
    info += "Endereço MAC: " + WiFi.macAddress() + "\n";
    info += "Endereço IP: " + WiFi.localIP().toString() + "\n";
    info += "RSSI WiFi: " + String(WiFi.RSSI()) + " dBm\n";
    info += "Servidor Django: " + String(DJANGO_SERVER_IP) + ":" + String(DJANGO_SERVER_PORT) + "\n";
    info += "Tag do dispositivo: " + String(DEVICE_TAG) + "\n\n";

    info += "----------------------------------------\n";
    info += "Status do Ar-condicionado:\n";
    info += "----------------------------------------\n";
    info += "Estado: " + String(arCondicionadoLigado ? "Ligado" : "Desligado") + "\n";
    info += "Temperatura configurada: " + String(temperaturaAtual) + "°C\n";
    info += "Temperatura ambiente: " + String(temperaturaAmbiente, 1) + "°C\n";
    info += "Modo: " + modoOperacao + "\n";
    info += "Velocidade: " + String(velocidadeVentilador) + "\n";
    info += "Swing: " + String(swingAtivado ? "Ativado" : "Desativado") + "\n";
    info += "Consumo: " + String(consumoEnergia, 2) + " kW/h\n\n";

    info += "----------------------------------------\n";
    info += "Sistema:\n";
    info += "----------------------------------------\n";
    info += "Tempo ligado: " + String(millis() / 1000) + " segundos\n";
    info += "Heap livre: " + String(ESP.getFreeHeap()) + " bytes\n";

    request->send(200, "text/plain", info);
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "Endpoint não encontrado.");
  });
}

void setupWebServer() {
  configurarEndpoints();
  // server.begin(); // O server.begin() deve ser chamado no setup() principal do firmware_base.ino
  Serial.println("Endpoints do servidor web configurados (via web_server_handler).");
}
