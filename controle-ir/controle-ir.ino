#include <Arduino.h>
#include <IRremote.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <stdlib.h> // Para alocação dinâmica de memória

#include "temperature_codes.h" // Arquivo que contém os códigos de frequência/temperatura

// ============= CONFIGURAÇÃO DO HARDWARE =============
// Pino do LED IR
const int PINO_IR = 23; // Ajuste conforme sua conexão

// Pino do LED de status
const int PINO_LED_STATUS = 2; // LED embutido do ESP32

// Pinos para sensores (simulados por enquanto)
const int PINO_SENSOR_TEMPERATURA = 36; // ADC
const int PINO_SENSOR_CORRENTE = 39; // ADC

// ============= CONFIGURAÇÃO DE REDE =============
// Variáveis de conexão Wi-Fi
const char* ssid = "Fabnet";      // Substitua pelo seu SSID
const char* senha = "71037523";    // Substitua pela sua senha

// Configuração de IP fixo (opcional)
IPAddress ip(192, 168, 1, 113);        // IP desejado para o ESP32
IPAddress gateway(192, 168, 1, 1);     // Gateway da sua rede
IPAddress subnet(255, 255, 255, 0);    // Máscara de sub-rede

// Endereço do servidor Django
const char* servidorDjango = "192.168.1.100";  // Substitua pelo IP do seu servidor Django
const int portaDjango = 8000;                  // Porta do servidor Django
const char* tagDispositivo = "ar-sala-reunioes"; // Identificador único deste dispositivo

// Servidor Web assíncrono
AsyncWebServer servidor(80);

// ============= VARIÁVEIS DE ESTADO DO AR-CONDICIONADO =============
uint8_t temperaturaAtual = 20;        // Temperatura inicial definida para 20°C
bool arCondicionadoLigado = false;    // Estado inicial do ar-condicionado
String modoOperacao = "cold";         // Modo inicial: refrigeração (cold, heat, fan)
uint8_t velocidadeVentilador = 1;     // Velocidade inicial: baixa (1-3)
bool swingAtivado = false;            // Oscilação desativada inicialmente

// ============= VARIÁVEIS PARA SENSORES =============
float temperaturaAmbiente = 25.0;     // Valor simulado para temperatura ambiente
float consumoEnergia = 0.0;           // Valor simulado para consumo de energia (kW/h)
float tensaoRede = 127.0;             // Tensão da rede elétrica em volts (ajuste conforme sua região)

// ============= VARIÁVEIS DE CONTROLE =============
unsigned long ultimaPing = 0;         // Último momento em que enviamos dados ao servidor
const unsigned long intervaloEnvioDados = 30000; // Enviar dados a cada 30 segundos

// Status da conexão WiFi
bool wifiConectado = false;

// ============= FUNÇÕES DE COMUNICAÇÃO IR =============

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
  
  // Zeramos o consumo quando o ar é desligado
  consumoEnergia = 0.0;
}

// Função para enviar comando de modo de operação
bool enviarComandoModo(String modo) {
  if (!arCondicionadoLigado) {
    Serial.println("Ar-condicionado está desligado. Ligue-o primeiro.");
    return false;
  }
  
  Serial.print("Enviando comando de modo: ");
  Serial.println(modo);
  
  // Esta função é um placeholder - você precisaria adicionar os códigos IR reais para os diferentes modos
  // Por agora apenas atualizamos o modo armazenado
  modoOperacao = modo;
  
  // Aqui você adicionaria o código real para enviar o comando IR
  Serial.println("Comando de modo enviado (simulado)");
  return true;
}

// Função para enviar comando de velocidade do ventilador
bool enviarComandoVelocidade(uint8_t velocidade) {
  if (!arCondicionadoLigado) {
    Serial.println("Ar-condicionado está desligado. Ligue-o primeiro.");
    return false;
  }
  
  if (velocidade < 1 || velocidade > 4) {
    Serial.println("Velocidade inválida. Use valores de 1 a 4.");
    return false;
  }
  
  Serial.print("Enviando comando de velocidade: ");
  Serial.println(velocidade);
  
  // Esta função é um placeholder - você precisaria adicionar os códigos IR reais
  velocidadeVentilador = velocidade;
  
  // Aqui você adicionaria o código real para enviar o comando IR
  Serial.println("Comando de velocidade enviado (simulado)");
  return true;
}

// Função para ativar/desativar o swing
bool enviarComandoSwing(bool ativar) {
  if (!arCondicionadoLigado) {
    Serial.println("Ar-condicionado está desligado. Ligue-o primeiro.");
    return false;
  }
  
  Serial.print("Enviando comando de swing: ");
  Serial.println(ativar ? "Ativar" : "Desativar");
  
  // Esta função é um placeholder - você precisaria adicionar os códigos IR reais
  swingAtivado = ativar;
  
  // Aqui você adicionaria o código real para enviar o comando IR
  Serial.println("Comando de swing enviado (simulado)");
  return true;
}

// ============= FUNÇÕES DE SENSORES =============

// Função para ler temperatura ambiente (simulada)
float lerTemperaturaAmbiente() {
  // Se tivesse um sensor real, você leria o valor aqui
  // Por enquanto, vamos simular uma temperatura variável
  if (arCondicionadoLigado) {
    // Se o ar está ligado, a temperatura tende a aproximar-se da temperatura definida
    float diferenca = temperaturaAmbiente - temperaturaAtual;
    temperaturaAmbiente -= diferenca * 0.05; // Ajuste lento em direção à temperatura definida
    
    // Adiciona um pouco de oscilação aleatória
    temperaturaAmbiente += random(-10, 10) / 100.0;
  } else {
    // Se o ar está desligado, a temperatura oscila em torno de um valor ambiente
    temperaturaAmbiente = 25.0 + random(-20, 20) / 10.0;  // 23.0 a 27.0
  }
  
  return temperaturaAmbiente;
}

// Função para calcular consumo de energia (simulado)
float calcularConsumoEnergia() {
  if (!arCondicionadoLigado) {
    return 0.0;
  }
  
  // Valores típicos para ar-condicionados:
  // - Split 9000 BTUs: ~800W = 0.8kWh
  // - Split 12000 BTUs: ~1100W = 1.1kWh
  
  // Base de consumo
  float consumoBase = 0.9; // kW/h para um ar médio
  
  // Ajusta com base na temperatura (quanto mais distante da ambiente, mais consome)
  float diferencaTemp = abs(temperaturaAmbiente - temperaturaAtual);
  float fatorTemp = 1.0 + (diferencaTemp / 10.0); // 10°C de diferença = 100% a mais
  
  // Ajusta com base no modo
  float fatorModo = 1.0;
  if (modoOperacao == "cold") {
    if (temperaturaAtual < temperaturaAmbiente) {
      fatorModo = 1.2; // Refrigeração consome mais
    }
  } else if (modoOperacao == "heat") {
    if (temperaturaAtual > temperaturaAmbiente) {
      fatorModo = 1.3; // Aquecimento consome ainda mais
    }
  } else if (modoOperacao == "fan") {
    fatorModo = 0.3; // Ventilação consome menos
  }
  
  // Ajusta com base na velocidade do ventilador
  float fatorVelocidade = 0.8 + (0.1 * velocidadeVentilador);
  
  // Calcula o consumo
  float consumo = consumoBase * fatorTemp * fatorModo * fatorVelocidade;
  
  // Adiciona alguma variação aleatória
  consumo += random(-5, 5) / 100.0;
  
  return max(consumo, 0.1); // No mínimo 0.1 kWh se estiver ligado
}

// ============= FUNÇÕES DE COMUNICAÇÃO COM O SERVIDOR =============

// Função para enviar dados ao servidor Django
void enviarDadosParaServidor() {
  if (!wifiConectado) {
    Serial.println("WiFi não conectado. Não é possível enviar dados.");
    return;
  }
  
  HTTPClient http;
  
  // URL do endpoint API no sistema Django
  String url = "http://";
  url += String(servidorDjango);
  url += ":";
  url += String(portaDjango);
  url += "/painelar/api/status/";
  
  Serial.print("Enviando dados para: ");
  Serial.println(url);
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  // Preparar o documento JSON
  DynamicJsonDocument doc(256);
  doc["tag"] = tagDispositivo;
  doc["temperatura_ambiente"] = temperaturaAmbiente;
  doc["consumo"] = consumoEnergia;
  doc["estado"] = arCondicionadoLigado;
  doc["temperatura"] = temperaturaAtual;
  doc["modo"] = modoOperacao;
  doc["velocidade"] = velocidadeVentilador;
  doc["swing"] = swingAtivado;
  
  String requestBody;
  serializeJson(doc, requestBody);
  
  // Enviar requisição
  int httpResponseCode = http.POST(requestBody);
  
  // Verificar resposta
  if (httpResponseCode > 0) {
    String resposta = http.getString();
    Serial.print("Resposta HTTP: ");
    Serial.println(httpResponseCode);
    Serial.print("Corpo da resposta: ");
    Serial.println(resposta);
    
    // Analisar a resposta para verificar se há comandos
    DynamicJsonDocument docResposta(512);
    DeserializationError error = deserializeJson(docResposta, resposta);
    
    if (!error) {
      String status = docResposta["status"];
      if (status == "success") {
        String comando = docResposta["comando"];
        
        if (comando != "none") {
          Serial.print("Comando recebido: ");
          Serial.println(comando);
          
          // Executar o comando recebido
          executarComando(comando);
        }
      }
    }
  } else {
    Serial.print("Erro na requisição HTTP: ");
    Serial.println(httpResponseCode);
  }
  
  http.end();
}

// Função para executar comandos recebidos do servidor
void executarComando(String comando) {
  // Verifica o tipo de comando
  if (comando == "ligar") {
    enviarComandoLigar();
  }
  else if (comando == "desligar") {
    enviarComandoDesligar();
  }
  else if (comando.startsWith("temperatura:")) {
    int novaTemp = comando.substring(12).toInt();
    if (novaTemp >= MIN_TEMPERATURE && novaTemp <= MAX_TEMPERATURE) {
      temperaturaAtual = novaTemp;
      enviarComandoTemperatura(temperaturaAtual);
    }
  }
  else if (comando.startsWith("modo:")) {
    String novoModo = comando.substring(5);
    enviarComandoModo(novoModo);
  }
  else if (comando.startsWith("velocidade:")) {
    int novaVelocidade = comando.substring(11).toInt();
    enviarComandoVelocidade(novaVelocidade);
  }
  else if (comando.startsWith("swing:")) {
    int estado = comando.substring(6).toInt();
    enviarComandoSwing(estado == 1);
  }
}

// ============= CONFIGURAÇÃO DE ENDPOINTS HTTP =============

void configurarEndpoints() {
  // Rota raiz - retorna informações básicas
  servidor.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String resposta = "ESP32 Controle AR v1.0\n";
    resposta += "Status: " + String(arCondicionadoLigado ? "Ligado" : "Desligado") + "\n";
    resposta += "Temperatura: " + String(temperaturaAtual) + "°C\n";
    resposta += "Modo: " + modoOperacao + "\n";
    request->send(200, "text/plain", resposta);
  });

  // Endpoint para ligar o ar-condicionado
  servidor.on("/ligar", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Recebida requisição: /ligar");
    enviarComandoLigar();
    request->send(200, "text/plain", "Ar-condicionado ligado.");
  });

  // Endpoint para desligar o ar-condicionado
  servidor.on("/desligar", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Recebida requisição: /desligar");
    enviarComandoDesligar();
    request->send(200, "text/plain", "Ar-condicionado desligado.");
  });

  // Endpoint para aumentar temperatura
  servidor.on("/aumentar", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Recebida requisição: /aumentar");
    aumentarTemperatura();
    request->send(200, "text/plain", "Temperatura aumentada.");
  });

  // Endpoint para diminuir temperatura
  servidor.on("/diminuir", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Recebida requisição: /diminuir");
    diminuirTemperatura();
    request->send(200, "text/plain", "Temperatura diminuída.");
  });

  // Endpoint para definir temperatura específica
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

  // Endpoint para verificar o status atual
  servidor.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Recebida requisição: /status");
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

  // Endpoint para obter apenas a temperatura atual
  servidor.on("/temperatura", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Recebida requisição: /temperatura");
    String resposta = "Temperatura atual: " + String(temperaturaAtual) + "°C.\n";
    resposta += "Temperatura ambiente: " + String(temperaturaAmbiente, 1) + "°C.\n";
    if (temperaturaAtual == MAX_TEMPERATURE) {
      resposta += "Temperatura máxima atingida.";
    }
    request->send(200, "text/plain", resposta);
  });

  // Endpoint para definir o modo de operação
  servidor.on("/modo", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Recebida requisição: /modo");
    if (request->hasParam("valor")) {
      String novoModo = request->getParam("valor")->value();
      if (novoModo == "cold" || novoModo == "heat" || novoModo == "fan" || novoModo == "dry" || novoModo == "auto") {
        if (enviarComandoModo(novoModo)) {
          String resposta = "Modo alterado para " + novoModo;
          request->send(200, "text/plain", resposta);
        } else {
          request->send(400, "text/plain", "Ar-condicionado está desligado. Ligue-o primeiro.");
        }
      } else {
        request->send(400, "text/plain", "Modo inválido. Use: cold, heat, fan, dry ou auto");
      }
    } else {
      request->send(400, "text/plain", "Parâmetro 'valor' não fornecido.");
    }
  });

  // Endpoint para definir a velocidade do ventilador
  servidor.on("/velocidade", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Recebida requisição: /velocidade");
    if (request->hasParam("valor")) {
      int velocidade = request->getParam("valor")->value().toInt();
      if (velocidade >= 1 && velocidade <= 4) {
        if (enviarComandoVelocidade(velocidade)) {
          String resposta = "Velocidade do ventilador alterada para " + String(velocidade);
          request->send(200, "text/plain", resposta);
        } else {
          request->send(400, "text/plain", "Ar-condicionado está desligado. Ligue-o primeiro.");
        }
      } else {
        request->send(400, "text/plain", "Velocidade inválida. Use valores de 1 a 4.");
      }
    } else {
      request->send(400, "text/plain", "Parâmetro 'valor' não fornecido.");
    }
  });

  // Endpoint para controlar o swing
  servidor.on("/swing", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Recebida requisição: /swing");
    if (request->hasParam("valor")) {
      int valor = request->getParam("valor")->value().toInt();
      bool ativar = (valor == 1);
      if (enviarComandoSwing(ativar)) {
        String resposta = "Swing " + String(ativar ? "ativado" : "desativado");
        request->send(200, "text/plain", resposta);
      } else {
        request->send(400, "text/plain", "Ar-condicionado está desligado. Ligue-o primeiro.");
      }
    } else {
      request->send(400, "text/plain", "Parâmetro 'valor' não fornecido.");
    }
  });

  // Endpoint para reiniciar o dispositivo
  servidor.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Reiniciando o dispositivo...");
    // Aguarda um pouco para enviar a resposta
    delay(500);
    ESP.restart();
  });

  // Endpoint para obter informações detalhadas
  servidor.on("/info", HTTP_GET, [](AsyncWebServerRequest *request){
    String info = "ESP32 Controle AR v1.0\n";
    info += "----------------------------------------\n";
    info += "Hardware e Configuração:\n";
    info += "----------------------------------------\n";
    info += "Endereço MAC: " + WiFi.macAddress() + "\n";
    info += "Endereço IP: " + WiFi.localIP().toString() + "\n";
    info += "RSSI WiFi: " + String(WiFi.RSSI()) + " dBm\n";
    info += "Servidor Django: " + String(servidorDjango) + ":" + String(portaDjango) + "\n";
    info += "Tag do dispositivo: " + String(tagDispositivo) + "\n\n";
    
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

  // Manejador para rotas não definidas
  servidor.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "Endpoint não encontrado.");
  });
}

// ============= FUNÇÃO SETUP (INICIALIZAÇÃO) =============
void setup() {
  // Inicializa porta serial
  Serial.begin(115200);
  Serial.println("\n----------------------------------------");
  Serial.println("ESP32 Controle AR - Iniciando...");
  Serial.println("----------------------------------------");

  // Inicializa o emissor IR
  IrSender.begin(PINO_IR);
  Serial.println("Emissor IR iniciado no pino " + String(PINO_IR));

  // Configura o LED de status
  pinMode(PINO_LED_STATUS, OUTPUT);
  digitalWrite(PINO_LED_STATUS, LOW);

  // Conecta na rede Wi-Fi
  WiFi.mode(WIFI_STA);
  
  // Opcionalmente usa IP fixo
  // WiFi.config(ip, gateway, subnet);
  
  WiFi.begin(ssid, senha);
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(ssid);

  unsigned long inicioTentativa = millis();
  const unsigned long tempoLimiteWiFi = 30000; // Timeout de 30 segundos

  // Pisca LED enquanto tenta conectar
  while (WiFi.status() != WL_CONNECTED && millis() - inicioTentativa < tempoLimiteWiFi) {
    digitalWrite(PINO_LED_STATUS, !digitalRead(PINO_LED_STATUS));
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    wifiConectado = true;
    Serial.print("Conectado ao Wi-Fi com o endereço IP: ");
    Serial.println(WiFi.localIP());
    
    // LED fica aceso quando conectado
    digitalWrite(PINO_LED_STATUS, HIGH);
  } else {
    wifiConectado = false;
    Serial.println("Falha ao conectar ao Wi-Fi. Tempo limite alcançado.");
    
    // LED apagado indica falha na conexão
    digitalWrite(PINO_LED_STATUS, LOW);
  }

  // Configura endpoints HTTP
  configurarEndpoints();

  // Inicia o servidor HTTP
  servidor.begin();
  Serial.println("Servidor HTTP iniciado na porta 80");
  
  // Log de inicialização concluída
  Serial.println("----------------------------------------");
  Serial.println("Inicialização concluída!");
  Serial.println("----------------------------------------");
}

// ============= FUNÇÃO LOOP (EXECUTADA CONTINUAMENTE) =============
void loop() {
  // Verifica a conexão WiFi
  if (WiFi.status() != WL_CONNECTED) {
    // Se perdeu a conexão mas estava conectado antes
    if (wifiConectado) {
      Serial.println("Conexão WiFi perdida. Tentando reconectar...");
      wifiConectado = false;
      
      // LED apagado indica desconexão
      digitalWrite(PINO_LED_STATUS, LOW);
      
      // Tenta reconectar
      WiFi.reconnect();
    }
  } else {
    // Se reconectou
    if (!wifiConectado) {
      Serial.print("WiFi reconectado. IP: ");
      Serial.println(WiFi.localIP());
      wifiConectado = true;
      
      // LED aceso indica reconexão
      digitalWrite(PINO_LED_STATUS, HIGH);
    }
  }

  // Atualiza leituras de sensores a cada 5 segundos
  static unsigned long ultimaLeitura = 0;
  if (millis() - ultimaLeitura > 5000) {
    ultimaLeitura = millis();
    
    // Atualiza leituras simuladas
    temperaturaAmbiente = lerTemperaturaAmbiente();
    consumoEnergia = calcularConsumoEnergia();
    
    // Log das leituras atuais
    Serial.println("----------------------------------------");
    Serial.println("Leituras atualizadas:");
    Serial.println("Temperatura ambiente: " + String(temperaturaAmbiente, 1) + "°C");
    Serial.println("Consumo estimado: " + String(consumoEnergia, 2) + " kW/h");
    Serial.println("----------------------------------------");
  }

  // Envia dados para o servidor a cada intervalo definido
  if (wifiConectado && millis() - ultimaPing > intervaloEnvioDados) {
    ultimaPing = millis();
    
    Serial.println("Enviando dados ao servidor...");
    enviarDadosParaServidor();
  }
  
  // Pisca LED rapidamente se o ar estiver ligado
  if (arCondicionadoLigado) {
    static unsigned long ultimoPisca = 0;
    if (millis() - ultimoPisca > 2000) {
      ultimoPisca = millis();
      
      // Pisca brevemente (apaga e acende)
      digitalWrite(PINO_LED_STATUS, LOW);
      delay(50);
      digitalWrite(PINO_LED_STATUS, HIGH);
    }
  }
}
