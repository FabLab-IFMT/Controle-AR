// firmware_base/src/ir_handler.h
#ifndef IR_HANDLER_H
#define IR_HANDLER_H

#include <Arduino.h>
#include <IRremoteESP8266.h> // Especificando a biblioteca mais comum para ESP32
// Ou #include <IRremote.h> se estiver usando a biblioteca padrão e ajustando para ESP32

// Declaração da instância do IRsender que será definida no .cpp
// ou no firmware_base.ino e declarada aqui como extern.
// Por simplicidade e encapsulamento, vamos supor que ela é gerenciada aqui.
// extern IRsend IrSender; // Se definida globalmente no .ino

void setupIR();
bool enviarComandoTemperaturaIR(uint8_t temperatura);
void enviarComandoLigarIR();
void enviarComandoDesligarIR();
bool enviarComandoModoIR(String modo);
bool enviarComandoVelocidadeIR(uint8_t velocidade);
bool enviarComandoSwingIR(bool ativar);

#endif
