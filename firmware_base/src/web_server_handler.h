// firmware_base/src/web_server_handler.h
#ifndef WEB_SERVER_HANDLER_H
#define WEB_SERVER_HANDLER_H

#include <ESPAsyncWebServer.h>

// A instância do servidor é geralmente global no .ino principal.
// Declaramos como extern aqui para que o .cpp possa configurá-la.
extern AsyncWebServer server;

void setupWebServer(); // Função para configurar todos os endpoints

#endif
