// firmware_base/src/api_client.cpp
#include "api_client.h"
#include "config.h"     // Para DJANGO_SERVER_IP, DJANGO_SERVER_PORT, DEVICE_TAG
#include "wifi_manager.h" // Para isWifiConnected()
#include "ir_handler.h"   // Para executar comandos recebidos do servidor (enviarComandoLigarIR, etc.)
#include "web_server_handler.h" // Para chamar funções como ligarArCondicionadoWeb que atualizam estado + enviam IR
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Variáveis globais de estado (declaradas 'extern' pois são definidas no firmware_base.ino)
// Necessárias para enviar o status atual ao servidor.
extern float temperaturaAmbiente;
extern float consumoEnergia;
extern bool arCondicionadoLigado;
extern uint8_t temperaturaAtual;
extern String modoOperacao;
extern uint8_t velocidadeVentilador;
extern bool swingAtivado;

// Função para executar comandos recebidos do servidor via API
// Esta função é chamada de dentro de enviarDadosParaServidorAPI se um comando for recebido.
void executarComandoRecebidoDaAPI(String comando) {
  Serial.print("API: Comando recebido do servidor: "); Serial.println(comando);

  // Aqui, em vez de chamar diretamente ir_handler, chamamos as funções do web_server_handler
  // que já cuidam da lógica de atualizar estado + chamar ir_handler.
  // Isso mantém a lógica de controle de estado mais centralizada.
  if (comando == "ligar") {
    // Idealmente, teríamos funções como ligarArCondicionadoEstado() que só mudam o estado,
    // e o web_server_handler e api_client chamariam essas + ir_handler.
    // Por simplicidade, vamos chamar as funções do web_server que já fazem isso.
    // Isso pode causar logs duplicados "Web:" e "API:", a serem refinados.
    ligarArCondicionadoWeb();
  } else if (comando == "desligar") {
    desligarArCondicionadoWeb();
  } else if (comando.startsWith("temperatura:")) {
    int novaTemp = comando.substring(12).toInt();
    if (novaTemp >= MIN_TEMPERATURE && novaTemp <= MAX_TEMPERATURE) { // MIN/MAX de config.h ou temperature_codes.h
      temperaturaAtual = novaTemp; // Atualiza estado
      enviarComandoTemperaturaIR(temperaturaAtual); // Envia comando IR
    }
  } else if (comando.startsWith("modo:")) {
    String novoModo = comando.substring(5);
    if (enviarComandoModoIR(novoModo)) { // Envia comando IR
        modoOperacao = novoModo; // Atualiza estado
    }
  } else if (comando.startsWith("velocidade:")) {
    int novaVelocidade = comando.substring(11).toInt();
     if (enviarComandoVelocidadeIR(novaVelocidade)) { // Envia comando IR
        velocidadeVentilador = novaVelocidade; // Atualiza estado
     }
  } else if (comando.startsWith("swing:")) {
    int estado = comando.substring(6).toInt();
    if (enviarComandoSwingIR(estado == 1)) { // Envia comando IR
        swingAtivado = (estado == 1); // Atualiza estado
    }
  } else {
    Serial.println("API: Comando desconhecido ou não implementado.");
  }
}


void enviarDadosParaServidorAPI() {
  if (!isWifiConnected()) {
    Serial.println("API: WiFi não conectado. Não é possível enviar dados.");
    return;
  }

  HTTPClient http;
  String url = "http://" + String(DJANGO_SERVER_IP) + ":" + String(DJANGO_SERVER_PORT) + "/painelar/api/status/";

  Serial.print("API: Enviando dados para: "); Serial.println(url);

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  DynamicJsonDocument doc(256); // Ajuste o tamanho conforme necessário
  doc["tag"] = DEVICE_TAG;
  doc["temperatura_ambiente"] = temperaturaAmbiente; // Idealmente arredondado para 1 casa decimal
  doc["consumo"] = consumoEnergia; // Idealmente arredondado para 2 casas decimais
  doc["estado"] = arCondicionadoLigado;
  doc["temperatura"] = temperaturaAtual;
  doc["modo"] = modoOperacao;
  doc["velocidade"] = velocidadeVentilador;
  doc["swing"] = swingAtivado;

  String requestBody;
  serializeJson(doc, requestBody);

  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode > 0) {
    String resposta = http.getString();
    Serial.print("API: Resposta HTTP: "); Serial.println(httpResponseCode);
    Serial.print("API: Corpo da resposta: "); Serial.println(resposta);

    DynamicJsonDocument docResposta(512); // Ajuste o tamanho
    DeserializationError error = deserializeJson(docResposta, resposta);

    if (!error) {
      const char* statusCmd = docResposta["status"]; // "status" da resposta, não confundir com estado do AC
      if (statusCmd && strcmp(statusCmd, "success") == 0) {
        const char* comando = docResposta["comando"];
        if (comando && strcmp(comando, "none") != 0) {
          executarComandoRecebidoDaAPI(comando);
        }
      } else {
        Serial.println("API: Resposta do servidor não foi 'success' ou campo 'status' ausente.");
      }
    } else {
      Serial.print("API: Erro ao deserializar JSON da resposta: "); Serial.println(error.c_str());
    }
  } else {
    Serial.print("API: Erro na requisição HTTP: "); Serial.println(HTTPClient::errorToString(httpResponseCode).c_str());
  }

  http.end();
}
