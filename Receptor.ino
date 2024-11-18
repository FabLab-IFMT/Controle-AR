#include <Arduino.h>
#include <IRremote.h>

const int RECV_PIN = 15;

void setup() {
  Serial.begin(115200);
  Serial.println("Receptor IR iniciado");

  IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK);
}

void loop() {
  if (IrReceiver.decode()) {
    Serial.println("Sinal IR recebido!");

    Serial.print("Protocolo: ");
    Serial.println(getProtocolString(IrReceiver.decodedIRData.protocol));

    Serial.print("Código em HEX: 0x");
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);

    Serial.print("Tamanho dos dados brutos: ");
    Serial.println(IrReceiver.decodedIRData.rawDataPtr->rawlen);

    Serial.println("Dados brutos (em microssegundos):");
    for (uint16_t i = 1; i < IrReceiver.decodedIRData.rawDataPtr->rawlen; i++) {
      unsigned int pulseDuration = IrReceiver.decodedIRData.rawDataPtr->rawbuf[i] * MICROS_PER_TICK;
      Serial.print(pulseDuration);
      Serial.print(", ");
    }
    Serial.println();

    IrReceiver.resume();
  }
}
