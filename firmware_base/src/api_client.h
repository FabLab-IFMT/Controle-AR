// firmware_base/src/api_client.h
#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <Arduino.h>

void enviarDadosParaServidorAPI();
// A função executarComandoAPI não é mais necessária aqui se o web_server_handler chama diretamente o ir_handler
// No entanto, se o servidor Django puder enviar comandos arbitrários que não apenas os de IR, ela pode ser útil.
// Por ora, vamos manter a lógica de execução de comando dentro do callback do HTTP POST.

#endif
