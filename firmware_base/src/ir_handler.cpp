// firmware_base/src/ir_handler.cpp
#include "ir_handler.h"
#include "config.h" // Para PINO_IR_TX e include de temperature_codes.h (que está dentro de config.h)
#include <IRremoteESP8266.h> // Para kIrLed e outros defines da lib, se necessário aqui
#include <Arduino.h> // Para Serial, malloc, free, memcpy_P, etc.

// Instância do IRsend. PINO_IR_TX é definido em config.h
// A biblioteca IRremoteESP8266 espera o pino como template argument ou no begin.
// Usar kIrLed como pino é uma opção da lib, mas PINO_IR_TX é mais flexível.
IRsend IrSender(PINO_IR_TX); // Ou o pino que você definiu em config.h

// Variáveis de estado do ar condicionado que este módulo precisa conhecer.
// Idealmente, o estado seria gerenciado de forma mais centralizada ou passada para as funções.
extern bool arCondicionadoLigado;    // Definida em firmware_base.ino
extern uint8_t temperaturaAtual;     // Definida em firmware_base.ino
// extern String modoOperacao;       // Definida em firmware_base.ino
// extern uint8_t velocidadeVentilador; // Definida em firmware_base.ino
// extern bool swingAtivado;         // Definida em firmware_base.ino


void setupIR() {
  IrSender.begin(); // Inicializa o emissor IR com o pino definido na instanciação
  Serial.println("Emissor IR iniciado no pino " + String(PINO_IR_TX) + " (via ir_handler)");
}

bool enviarComandoTemperaturaIR(uint8_t temperatura) {
  if (!arCondicionadoLigado) {
    Serial.println("IR: Ar-condicionado está desligado. Ligue-o primeiro.");
    return false;
  }
  Serial.print("IR: Enviando comando de temperatura: "); Serial.println(temperatura);

  for (int i = 0; i < sizeof(temperatureCodes) / sizeof(temperatureCodes[0]); i++) {
    if (temperatureCodes[i].temperature == temperatura) {
      // Alocar memória para o buffer Raw
      uint16_t* buffer = (uint16_t*)malloc(temperatureCodes[i].length * sizeof(uint16_t));
      if (buffer == nullptr) {
        Serial.println("IR: Erro ao alocar memória para o buffer");
        return false;
      }
      // Copiar os dados raw da PROGMEM para o buffer
      memcpy_P(buffer, temperatureCodes[i].rawData, temperatureCodes[i].length * sizeof(uint16_t));

      IrSender.sendRaw(buffer, temperatureCodes[i].length, 38); // 38kHz é comum
      free(buffer); // Liberar memória

      Serial.print("IR: Comando de temperatura enviado: "); Serial.print(temperatura); Serial.println("°C");
      return true;
    }
  }
  Serial.println("IR: Temperatura não encontrada nos códigos armazenados.");
  return false;
}

void enviarComandoLigarIR() {
  Serial.println("IR: Enviando comando 'Ligar'");
  // arCondicionadoLigado = true; // O estado deve ser atualizado pelo chamador ou gerenciador de estado
  // temperaturaAtual = 20; // Define uma temperatura padrão ao ligar, gerenciado pelo chamador

  uint16_t* buffer = (uint16_t*)malloc(onCommand20.length * sizeof(uint16_t));
  if (buffer == nullptr) {
    Serial.println("IR: Erro ao alocar memória para o buffer (Ligar)");
    return;
  }
  memcpy_P(buffer, onCommand20.rawData, onCommand20.length * sizeof(uint16_t));
  IrSender.sendRaw(buffer, onCommand20.length, 38);
  free(buffer);
  Serial.println("IR: Comando 'Ligar' enviado (para 20°C por padrão).");
}

void enviarComandoDesligarIR() {
  Serial.println("IR: Enviando comando 'Desligar'");
  // arCondicionadoLigado = false; // O estado deve ser atualizado pelo chamador

  uint16_t* buffer = (uint16_t*)malloc(offCommand.length * sizeof(uint16_t));
  if (buffer == nullptr) {
    Serial.println("IR: Erro ao alocar memória para o buffer (Desligar)");
    return;
  }
  memcpy_P(buffer, offCommand.rawData, offCommand.length * sizeof(uint16_t));
  IrSender.sendRaw(buffer, offCommand.length, 38);
  free(buffer);
  Serial.println("IR: Comando 'Desligar' enviado");
}

// Placeholder para as demais funções. Implementar similarmente a enviarComandoTemperaturaIR
bool enviarComandoModoIR(String modo) {
  if (!arCondicionadoLigado) return false;
  Serial.println("IR: Comando MODO " + modo + " (TODO: Implementar envio real)");
  // Aqui viria a lógica para encontrar o raw code do modo e enviar
  return true; // Simulado
}

bool enviarComandoVelocidadeIR(uint8_t velocidade) {
  if (!arCondicionadoLigado) return false;
  Serial.println("IR: Comando VELOCIDADE " + String(velocidade) + " (TODO: Implementar envio real)");
  // Aqui viria a lógica para encontrar o raw code da velocidade e enviar
  return true; // Simulado
}

bool enviarComandoSwingIR(bool ativar) {
  if (!arCondicionadoLigado) return false;
  Serial.println("IR: Comando SWING " + String(ativar) + " (TODO: Implementar envio real)");
  // Aqui viria a lógica para encontrar o raw code do swing e enviar
  return true; // Simulado
}
