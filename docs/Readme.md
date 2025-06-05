# Controle-AR Multi-Dispositivo

Este projeto tem como objetivo controlar um ar condicionado utilizando um microcontrolador ESP32, permitindo a integração com uma aplicação web em Django. A estrutura foi projetada para suportar múltiplos dispositivos com configurações individualizadas.

## Estrutura do Projeto

O projeto está organizado da seguinte forma:

```
.
├── LICENSE
├── docs/
│   └── Readme.md         # Esta documentação
├── firmware_base/
│   └── src/
│       ├── firmware_base.ino     # Código principal e lógica do firmware
│       └── temperature_codes.h # Códigos IR para controle de temperatura (comum a todos)
├── sala_aula_1_device/   # Exemplo de pasta para um dispositivo específico
│   ├── sala_aula_1_device.ino  # Arquivo principal do sketch para este dispositivo
│   └── config.h              # Configurações específicas para sala_aula_1
├── sala_reunioes_device/ # Exemplo de pasta para outro dispositivo
│   ├── sala_reunioes_device.ino
│   └── config.h
└── ...                     # Outras pastas de dispositivos
```

- **\`firmware_base/src/\`**: Contém o código fonte principal (\`firmware_base.ino\`) que implementa todas as funcionalidades de controle do ar condicionado, servidor web embarcado, e comunicação com o backend Django. Também inclui \`temperature_codes.h\` que armazena os códigos infravermelhos (estes podem ser movidos para dentro de cada \`config.h\` de dispositivo se os códigos variarem muito entre modelos de ar condicionado).
- **Pastas de Dispositivos (ex: \`sala_aula_1_device/\`, \`sala_reunioes_device/\`)**: Cada pasta representa um microcontrolador físico.
    - \`[nome_do_dispositivo].ino\`: É o arquivo de sketch principal que você abrirá no Arduino IDE (ou configurará no PlatformIO) para compilar e enviar o firmware para aquele dispositivo específico. Ele é mínimo e geralmente apenas inclui o \`config.h\` local.
    - \`config.h\`: Contém todas as configurações específicas para aquele dispositivo, como credenciais Wi-Fi, endereço IP (se estático), tag do dispositivo para o servidor Django, pinos de hardware (se diferirem do padrão em \`firmware_base.ino\`), etc. O \`firmware_base.ino\` é escrito para usar as macros definidas neste arquivo.

## Como Funciona a Configuração por Dispositivo

O \`firmware_base.ino\` inclui um arquivo chamado \`config.h\`. Quando você compila o sketch a partir de uma pasta de dispositivo (ex: \`sala_aula_1_device/sala_aula_1_device.ino\`), o pré-processador do Arduino/PlatformIO prioriza o \`config.h\` localizado na mesma pasta do sketch principal. Desta forma, o código genérico em \`firmware_base.ino\` é parametrizado com as configurações do dispositivo específico que está sendo compilado.

## Adicionando um Novo Dispositivo

1. Crie uma nova pasta para o seu dispositivo, por exemplo, \`nova_sala_device/\`.
2. Dentro de \`nova_sala_device/\`, crie um arquivo \`config.h\`. Você pode copiar um \`config.h\` existente de outro dispositivo como template e ajustar os valores (SSID, senha, \`DEVICE_TAG\`, pinos, etc.) para o novo dispositivo.
3. Crie um arquivo \`nova_sala_device.ino\` dentro de \`nova_sala_device/\`. O conteúdo mínimo para este arquivo é:
   \`\`\`cpp
   // Firmware para Nova Sala Device
   #include "config.h" // Inclui as configurações específicas deste dispositivo

   // O Arduino IDE/PlatformIO irá compilar e linkar o firmware_base.ino automaticamente.
   // Certifique-se de que a estrutura de pastas permita que o compilador encontre firmware_base/src/firmware_base.ino
   // ou ajuste os caminhos de inclusão/biblioteca no seu ambiente de build (ex: platformio.ini).
   \`\`\`
4. Se o novo dispositivo utilizar códigos IR completamente diferentes que não podem ser parametrizados em \`temperature_codes.h\`, você pode optar por copiar \`temperature_codes.h\` para dentro da pasta do novo dispositivo e modificar o \`#include\` no \`config.h\` do dispositivo para apontar para a cópia local.

## Compilando e Enviando o Firmware

**Usando o Arduino IDE:**
1. Abra o arquivo \`.ino\` principal da pasta do dispositivo desejado (ex: \`sala_aula_1_device/sala_aula_1_device.ino\`).
2. Certifique-se de que a placa ESP32 correta e a porta serial estão selecionadas no menu "Ferramentas".
3. Compile e envie o sketch.

**Usando PlatformIO:**
1. Você precisaria configurar um arquivo \`platformio.ini\` na raiz do projeto ou em cada pasta de dispositivo para definir ambientes separados.
2. Cada ambiente especificaria o \`src_dir\` para a pasta do dispositivo e possivelmente usaria \`build_flags\` para garantir que o \`config.h\` correto seja priorizado ou para definir macros de configuração.
3. Compile e envie usando os comandos do PlatformIO (ex: \`pio run -e sala_aula_1_env -t upload\`).
*Nota: A configuração detalhada do PlatformIO para esta estrutura não está incluída neste Readme, mas é uma melhoria recomendada para projetos maiores.*

## Hardware (Exemplo)

- ESP32
- Sensor IR (genérico para emissão)
- Transistor (ex: BC547B) e resistores para acionar o LED IR com segurança
- Módulo DHT-11 (ou similar) para leitura de temperatura e umidade ambiente (opcional, já que o código atual simula)
Os pinos exatos para IR, LED de status, etc., são definidos no arquivo \`config.h\` de cada dispositivo.

## Bibliotecas Necessárias (para \`firmware_base.ino\`)

- [IRremoteESP8266](https://github.com/crankyoldgit/IRremoteESP8266) (ou [Arduino-IRremote](https://github.com/Arduino-IRremote/Arduino-IRremote) se ajustado, mas a primeira é mais comum para ESP32)
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [AsyncTCP](https://github.com/me-no-dev/AsyncTCP) (dependência do ESPAsyncWebServer)
- [ArduinoJson](https://arduinojson.org/) (para comunicação com o servidor Django)

## Licença

Este projeto está licenciado sob a [GPL-3.0 License](../../LICENSE).
