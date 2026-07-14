# ⚡ CTT — Controle de Temperatura para Transformadores

Sistema embarcado baseado em **ESP8266** para monitoramento elétrico (tensão, corrente, potência, frequência e fator de potência via sensor **PZEM-004T v3**) com **controle automático/manual de relé** e **estimativa de temperatura do enrolamento** de transformadores a óleo, tudo acessível por uma interface web própria criada pelo ESP em modo Access Point — sem depender de roteador ou internet.

---

## 📋 Índice

- [Visão geral](#-visão-geral)
- [Funcionalidades](#-funcionalidades)
- [Hardware necessário](#-hardware-necessário)
- [Esquema de ligação](#-esquema-de-ligação)
- [Estrutura do projeto](#-estrutura-do-projeto)
- [Bibliotecas necessárias](#-bibliotecas-necessárias)
- [Como gravar no ESP8266](#-como-gravar-no-esp8266)
- [Como usar](#-como-usar)
- [API HTTP](#-api-http)
- [Cálculo de temperatura do enrolamento](#-cálculo-de-temperatura-do-enrolamento)
- [Limitações conhecidas](#-limitações-conhecidas)
- [Solução de problemas](#-solução-de-problemas)
- [Licença](#-licença)

---

## 🔎 Visão geral

O CTT cria sua própria rede Wi-Fi (modo **Access Point**), então qualquer celular ou notebook pode se conectar diretamente nela e acessar `http://192.168.4.1` para:

- Ver tensão, corrente, potência, frequência e fator de potência em tempo real
- Ligar/desligar o relé manualmente, ou deixar em modo automático (por corrente de carga)
- Configurar o limite de corrente e o tempo de desligamento do modo automático
- Estimar a temperatura do enrolamento do transformador a partir da corrente medida
- Consultar o histórico de eventos (ligou/desligou o relé, trocou de modo)

Não é necessário internet nem roteador — o ESP8266 e o dispositivo que acessa a página formam uma rede isolada.

## ✨ Funcionalidades

- 📶 **Access Point próprio**, IP fixo `192.168.4.1`
- 📊 **Leitura em tempo real** via PZEM-004T v3 (tensão, corrente, potência, energia, frequência, fator de potência)
- ⚙️ **Modo automático**: liga o relé quando detecta corrente de carga, desliga após um tempo configurável sem carga
- 🔧 **Modo manual**: liga/desliga o relé sob demanda
- 🌡️ **Estimativa de temperatura do enrolamento** (baseada na norma IEC 60076-7, a partir da corrente e da temperatura do óleo informada)
- 📋 **Histórico de eventos** com data/hora real (sincronizada com o dispositivo que acessa a página)
- 🖥️ **Interface web responsiva**, funciona bem em celular

## 🔧 Hardware necessário

| Componente | Observação |
|---|---|
| ESP8266 (NodeMCU, Wemos D1 mini, etc.) | Controlador principal |
| PZEM-004T v3 | Sensor de tensão/corrente/potência |
| Módulo relé (5V ou 3.3V, conforme sua carga) | Aciona a carga controlada |
| Fonte de alimentação adequada para o ESP e o relé | |

## 🔌 Esquema de ligação

### PZEM-004T v3 → ESP8266 (Hardware Serial, UART0)

| PZEM | ESP8266 |
|---|---|
| TX  | **GPIO3** (RX) |
| RX  | **GPIO1** (TX) |
| GND | GND (comum obrigatório) |

> ⚠️ **Importante:** o PZEM é ligado no UART0 **padrão** do ESP8266 (os mesmos pinos do conversor USB-serial da placa), **sem usar `Serial.swap()`**. Isso é proposital: os pinos alternativos do swap (GPIO13/GPIO15) incluem o **GPIO15**, que é um pino de *strapping* usado pelo ESP8266 para decidir o modo de boot. Se o PZEM for energizado ao mesmo tempo que o ESP e mantiver esse pino em nível alto durante o boot, o ESP pode falhar ao iniciar (trava, ou entra em modo de programação). Usando os pinos padrão (GPIO1/GPIO3, que não são pinos de boot), esse risco não existe — em compensação, o monitor serial via USB não pode ser usado ao mesmo tempo que o PZEM está conectado. Mensagens de debug, quando necessárias, saem pelo `Serial1` (GPIO2, apenas transmissão).

### Relé → ESP8266

| Relé | ESP8266 |
|---|---|
| Sinal (IN) | GPIO5 |
| GND | GND |
| VCC | Conforme especificação do módulo do relé |

## 📁 Estrutura do projeto

```
CTT_Energia/
├── monitor_energia_rele.ino   # Firmware principal (lógica, servidor web, rotas HTTP)
└── webpage_html.h              # Interface web (HTML/CSS/JS) embutida na flash via PROGMEM
```

Os dois arquivos precisam estar **na mesma pasta**, e essa pasta precisa ter **o mesmo nome do arquivo `.ino`** (regra do Arduino IDE). O `webpage_html.h` aparece automaticamente como uma segunda aba dentro do sketch.

> O HTML fica separado do `.ino` de propósito: o Arduino IDE gera protótipos de função automaticamente varrendo o `.ino` como texto puro, e sem essa separação ele confundia `function nome() {` do JavaScript embutido com uma função C++, quebrando a compilação.

## 📚 Bibliotecas necessárias

Instale pela IDE Arduino (Gerenciador de Bibliotecas) ou PlatformIO:

- `ESP8266WiFi` (já vem com o core do ESP8266)
- [`ESPAsyncWebServer`](https://github.com/me-no-dev/ESPAsyncWebServer)
- [`ESPAsyncTCP`](https://github.com/me-no-dev/ESPAsyncTCP) (dependência do ESPAsyncWebServer no ESP8266)
- [`PZEM004Tv30`](https://github.com/mandulaj/PZEM-004T-v30)
- [`ArduinoJson`](https://arduinojson.org/) (v6 ou v7)

## 🚀 Como gravar no ESP8266

1. Abra `monitor_energia_rele.ino` na Arduino IDE (o `webpage_html.h` abre junto, como aba).
2. Instale as bibliotecas listadas acima.
3. Selecione a placa correta (NodeMCU 1.0, Wemos D1 mini, etc.) em **Ferramentas > Placa**.
4. Selecione a porta serial correspondente.
5. Clique em **Carregar** (upload).
6. Após o boot, procure pela rede Wi-Fi **`Monitor_Energia`** e conecte-se a ela.
7. Abra `http://192.168.4.1` no navegador.

## 🖱️ Como usar

Ao conectar, a página abre no menu principal com quatro seções:

- **📊 Monitoramento** — leituras em tempo real e estado do relé/modo
- **⚙️ Configuração** — ajusta o limite de corrente e o tempo de desligamento do modo automático
- **🌡️ Temperatura** — informe a temperatura do óleo e veja a estimativa do enrolamento
- **📋 Histórico** — lista de eventos (ligou/desligou, trocou de modo) com data e hora reais

O modo **manual** libera os botões de ligar/desligar o relé; no modo **automático**, o relé é controlado sozinho pela lógica de corrente/tempo configurada.

## 🌐 API HTTP

Todas as respostas são em JSON.

| Rota | Método | Descrição |
|---|---|---|
| `/` | GET | Serve a interface web |
| `/status` | GET | Retorna tensão, corrente, potência, energia, frequência, fator de potência, estado do relé/modo e flag de erro do sensor |
| `/rele/on` | GET | Liga o relé (somente em modo manual) |
| `/rele/off` | GET | Desliga o relé (somente em modo manual) |
| `/modo/manual` | GET | Ativa o modo manual |
| `/modo/auto` | GET | Ativa o modo automático |
| `/config?limite=X&tempo=Y` | GET | Define o limite de corrente (A) e o tempo de desligamento (min) do modo automático |
| `/setclock?agora=<epoch_ms>` | GET | Sincroniza o relógio interno do ESP com a hora real do dispositivo conectado (usado para as datas do histórico) |
| `/historico` | GET | Retorna a lista de eventos registrados, com data/hora real já calculada |

## 🌡️ Cálculo de temperatura do enrolamento

A estimativa usa uma simplificação da norma **IEC 60076-7**:

```
ΔT = H × (I / Iₙ)^1.6
Temperatura do enrolamento = Temperatura do óleo + ΔT
```

Onde `H` é o gradiente térmico nominal (valor típico de 12 °C para transformadores a óleo) e `Iₙ` é a corrente nominal do transformador. **Ajuste `correnteNominal` no JavaScript conforme a especificação do seu transformador** — o valor padrão (100 A) é apenas um exemplo.

## ⚠️ Limitações conhecidas

- **Sem RTC (relógio de tempo real):** o ESP8266 não guarda a hora sozinho. A data/hora do histórico só fica correta depois que algum dispositivo se conecta à página pelo menos uma vez (a sincronização é automática ao abrir a interface).
- **Sem segurança na rede Wi-Fi/API:** por definição de projeto, a rede e as rotas HTTP não implementam autenticação real. Não exponha esse dispositivo em uma rede compartilhada ou pública sem avaliar os riscos.
- **Modo automático:** o ESP não distingue entre "carga desligada" e "sensor com falha" além do que o PZEM reporta — leituras `NaN` são tratadas como corrente zero.

## 🛠️ Solução de problemas

**A página não abre em `192.168.4.1`**
Confirme que está conectado à rede Wi-Fi `Monitor_Energia` (não à sua rede doméstica) e que o firmware foi gravado com o `webpage_html.h` presente na mesma pasta do `.ino`.

**Erro de compilação `'function' does not name a type`**
Verifique se o HTML está no arquivo `webpage_html.h` separado (não colado direto no `.ino`) — esse erro acontece quando o Arduino IDE tenta gerar protótipos de função varrendo JavaScript como se fosse C++.

**Datas no histórico aparecendo como 31/12/1969**
Acesse a página normalmente pelo navegador; a sincronização de relógio acontece automaticamente ao carregar. Eventos registrados antes da primeira sincronização são corrigidos retroativamente assim que ela acontece.

**O ESP não sobe a rede Wi-Fi / trava ao ligar com o PZEM já energizado**
Confirme que o PZEM está ligado nos pinos padrão do UART0 (GPIO1/GPIO3), **não** nos pinos alternativos do swap (GPIO13/GPIO15) — ver seção [Esquema de ligação](#-esquema-de-ligação).

## 📄 Licença

Defina aqui a licença do projeto (ex.: MIT, GPL-3.0) conforme sua preferência.

---

Desenvolvido por: **David Benne**
