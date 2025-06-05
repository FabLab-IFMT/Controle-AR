// Firmware para Sala de Reuniões
#include "config.h" // Configurações específicas deste dispositivo

// O código principal está em firmware_base.ino
// O compilador Arduino geralmente compila todos os .ino na pasta do projeto principal
// e também os .ino e .cpp/.h de subpastas como 'src' ou bibliotecas linkadas.
// Se firmware_base.ino está em ../firmware_base/src/firmware_base.ino,
// o Arduino IDE (quando o sketch principal é sala_reunioes_device.ino)
// irá compilar firmware_base.ino e linká-lo.
// A inclusão de "config.h" dentro de firmware_base.ino irá, por padrão do C++,
// procurar primeiro no diretório do arquivo que está sendo compilado (firmware_base/src)
// e depois nos caminhos de inclusão do projeto.
// Para que o firmware_base.ino use o config.h específico do dispositivo,
// o caminho para o config.h do dispositivo precisa ser adicionado aos caminhos de
// inclusão globais do compilador, ou o firmware_base.ino precisa ser modificado
// para incluir algo como "../config.h" (se compilado de dentro de 'src')
// ou "device_config.h" e cada sketch principal fornece este arquivo.

// A abordagem mais comum com o Arduino IDE é:
// 1. O sketch principal (este arquivo) inclui seu "config.h".
// 2. O código compartilhado (firmware_base.ino) é compilado junto.
// 3. O código compartilhado usa as macros (WIFI_SSID, etc.) definidas pelo "config.h"
//    que foi incluído pelo sketch principal. O #include "config.h" no firmware_base.ino
//    é mais para clareza ou para builds fora do Arduino IDE.

// Para este exercício, assumimos que o firmware_base.ino agora espera que as configs
// (macros) estejam definidas quando ele for compilado. Este arquivo garante que
// config.h (que define as macros) é incluído.

// Não é estritamente necessário incluir firmware_base.ino aqui se o Arduino IDE
// estiver compilando todos os arquivos .ino na pasta do sketch e suas subpastas.
// No entanto, para clareza ou para alguns sistemas de build, pode ser útil.
// #include "../firmware_base/src/firmware_base.ino"
// Se firmware_base.ino contiver setup() e loop(), não deve ser incluído diretamente
// assim, pois causaria definições múltiplas. O Arduino IDE lida com isso.

void setup() {
  // O setup() principal está em firmware_base.ino
  // Ele usará as configurações de config.h
}

void loop() {
  // O loop() principal está em firmware_base.ino
}
