/*****************************************************************************************
 *  PROJETO: Sistema de Monitoramento Elétrico com Relé
 *  CONTROLADOR: ESP8266
 *
 *  O QUE ESTE CÓDIGO FAZ?
 *  - Cria uma rede Wi-Fi própria (Access Point)
 *  - Disponibiliza uma interface WEB via navegador
 *  - Mede tensão, corrente, potência, frequência e fator de potência (PZEM)
 *  - Controla um relé em modo MANUAL ou AUTOMÁTICO
 *  - Registra eventos importantes em um histórico interno
 *
 *  REVISÃO: PZEM migrado de SoftwareSerial para HARDWARE SERIAL (UART0 com swap),
 *  além de correções de bugs encontrados no código original. Ver comentários
 *  marcados com "// CORREÇÃO:" para entender cada mudança.
 *
 *  ESTE CÓDIGO ESTÁ TOTALMENTE COMENTADO PARA FINS DIDÁTICOS
 *****************************************************************************************/


/* ======================== BIBLIOTECAS ======================== */

#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <PZEM004Tv30.h>
#include <ArduinoJson.h>

// CORREÇÃO: SoftwareSerial removido. O PZEM agora usa o UART0 (Serial) real do ESP8266.
// #include <SoftwareSerial.h>  // não é mais necessário


/* ======================== DEFINIÇÕES DE HARDWARE ======================== */

// Pino onde o relé está conectado
const int PINO_RELE = 5;

// CORREÇÃO (v2): voltamos a usar o UART0 nos pinos PADRÃO (GPIO1=TX / GPIO3=RX),
// SEM Serial.swap(). Motivo: o GPIO15 é um pino de "strapping" (junto com
// GPIO0 e GPIO2) que o ESP8266 lê no instante do boot pra decidir o modo de
// inicialização. Ele precisa estar em LOW nesse instante; se o PZEM for
// energizado ao mesmo tempo que o ESP, o pino de RX dele pode segurar o
// GPIO15 em HIGH bem na janela do boot, fazendo o ESP entrar em modo de
// download/programação em vez de bootar normal (o ESP "trava" e o Wi-Fi
// nunca sobe). Usando os pinos padrão do UART0 (que não são strapping pins),
// esse risco desaparece — o único efeito colateral é perder o monitor serial
// via USB enquanto o PZEM estiver conectado (o debug continua saindo pelo
// Serial1, no GPIO2, que fica livre e intacto).
PZEM004Tv30 pzem(Serial);

// Criação do servidor WEB na porta 80 (padrão HTTP)
AsyncWebServer server(80);


/* ======================== VARIÁVEIS DE ESTADO ======================== */

bool releLigado = false;
bool modoAutomatico = true;
bool temporizadorAtivo = false;

// Corrente mínima para o relé ligar automaticamente
float correnteLimite = 1.0;

// Tempo (em minutos) que o relé permanece ligado após perda de carga
int tempoDesligamento = 40;

unsigned long inicioTemporizador = 0;


/* ======================== VARIÁVEIS DE MEDIÇÃO ======================== */

float tensao = 0;
float corrente = 0;
float potencia = 0;
float energia = 0;
float frequencia = 0;
float fatorPotencia = 0;


/* ======================== HISTÓRICO DE EVENTOS ======================== */

struct Evento {
  unsigned long tempo;
  bool estadoRele;
  bool modoAuto;
  float correnteEvento;
};

#define MAX_EVENTOS 50

Evento historico[MAX_EVENTOS];
int indiceHistorico = 0;

// CORREÇÃO: contador de quantos eventos já foram realmente preenchidos.
// Sem isso, /historico retornava posições "zeradas" (lixo) antes do buffer encher.
int totalEventosRegistrados = 0;

// CORREÇÃO: o ESP8266 não tem RTC, então historico[i].tempo guarda apenas
// millis() (tempo desde o boot). relogioOffsetMs guarda a diferença entre a
// hora real (recebida do navegador via /setclock) e o millis() no momento da
// sincronização. Somando os dois, reconstruímos a data/hora real de qualquer
// evento, mesmo os que foram registrados antes da sincronização acontecer.
// Valor -1 = ainda não sincronizado (nenhum navegador conectou ainda).
double relogioOffsetMs = -1;


/* ======================== FUNÇÕES AUXILIARES ======================== */

// CORREÇÃO: sanitiza valores NaN do PZEM (sensor desconectado / falha de leitura).
// NaN não é um token JSON válido e quebra o JSON.parse() no navegador.
float valorSeguro(float v) {
  return isnan(v) ? 0.0 : v;
}


/* ======================== FUNÇÃO: REGISTRAR EVENTO ======================== */

void registrarEvento() {
  historico[indiceHistorico].tempo = millis();
  historico[indiceHistorico].estadoRele = releLigado;
  historico[indiceHistorico].modoAuto = modoAutomatico;
  // CORREÇÃO: salva a corrente já sanitizada (sem NaN) no histórico.
  historico[indiceHistorico].correnteEvento = valorSeguro(corrente);

  indiceHistorico = (indiceHistorico + 1) % MAX_EVENTOS;

  if (totalEventosRegistrados < MAX_EVENTOS) {
    totalEventosRegistrados++;
  }
}


/* ======================== FUNÇÕES DE CONTROLE DO RELÉ ======================== */

void ligarRele() {
  releLigado = true;
  digitalWrite(PINO_RELE, HIGH);
  registrarEvento();
}

void desligarRele() {
  releLigado = false;
  digitalWrite(PINO_RELE, LOW);
  registrarEvento();
}


#include "webpage_html.h"



/* ======================== SETUP (EXECUTA UMA VEZ) ======================== */

void setup() {

  // CORREÇÃO: Serial1 (GPIO2, TX-only) fica reservado para debug via monitor serial
  // externo (não dá pra usar o monitor da IDE Arduino nele, pois não é a porta USB).
  Serial1.begin(115200);
  Serial1.println("Iniciando sistema...");

  pinMode(PINO_RELE, OUTPUT);
  desligarRele();

  // CORREÇÃO (v2): PZEM em hardware serial nos pinos PADRÃO do UART0
  // (GPIO1=TX / GPIO3=RX), sem swap — ver explicação completa acima, no
  // ponto onde o objeto "pzem" é criado. Baud rate do PZEM004T v3 é 9600.
  Serial.begin(9600);

  /* -------- CONFIGURAÇÃO DO WI-FI -------- */

  IPAddress ip(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  // CORREÇÃO: a ordem correta é primeiro subir o Access Point e DEPOIS aplicar o
  // IP fixo. Do jeito original (softAPConfig antes de softAP), a configuração de
  // IP podia ser sobrescrita/ignorada quando o AP subia.
  WiFi.softAP("Monitor_Energia", "12345678");
  WiFi.softAPConfig(ip, gateway, subnet);

  /* -------- ROTAS HTTP -------- */

  // CORRECAO: rota "/" que faltava -- serve a pagina principal.
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", PAGINA_HTML);
  });

  // Retorna o estado geral do sistema
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){

    DynamicJsonDocument doc(512);

    doc["rele"] = releLigado;
    doc["automatico"] = modoAutomatico;
    // CORREÇÃO: todos os valores passam por valorSeguro() antes de ir pro JSON,
    // evitando "NaN" no corpo da resposta quando o PZEM não responde.
    doc["tensao"] = valorSeguro(tensao);
    doc["corrente"] = valorSeguro(corrente);
    doc["potencia"] = valorSeguro(potencia);
    doc["energia"] = valorSeguro(energia);       // CORREÇÃO: energia agora é exposta
    doc["frequencia"] = valorSeguro(frequencia);
    doc["fp"] = valorSeguro(fatorPotencia);
    // CORRECAO: o front-end ja lia "erroSensor" mas o backend nunca enviava esse campo.
    doc["erroSensor"] = isnan(tensao) || isnan(corrente) || isnan(potencia) ||
                         isnan(frequencia) || isnan(fatorPotencia);
    doc["limite_corrente"] = correnteLimite;
    doc["tempo_desligamento_min"] = tempoDesligamento;

    String resposta;
    serializeJson(doc, resposta);

    request->send(200, "application/json", resposta);
  });

  // Liga o relé manualmente
  server.on("/rele/on", HTTP_GET, [](AsyncWebServerRequest *request){
    // CORREÇÃO: só liga (e só registra evento) se realmente estava desligado,
    // evitando eventos duplicados no histórico a cada clique repetido.
    if (!modoAutomatico && !releLigado) ligarRele();
    // CORRECAO: respondia em text/plain, mas o front-end sempre faz resposta.json(),
    // o que gerava o erro "unexpected token 'R' ... is not valid JSON".
    request->send(200, "application/json", "{\"mensagem\":\"RELE LIGADO\"}");
  });

  // Desliga o relé manualmente
  server.on("/rele/off", HTTP_GET, [](AsyncWebServerRequest *request){
    // CORREÇÃO: mesma lógica, evita registro duplicado.
    if (!modoAutomatico && releLigado) desligarRele();
    request->send(200, "application/json", "{\"mensagem\":\"RELE DESLIGADO\"}");
  });

  // Ativa modo manual
  server.on("/modo/manual", HTTP_GET, [](AsyncWebServerRequest *request){
    if (modoAutomatico) {
      modoAutomatico = false;
      temporizadorAtivo = false; // CORREÇÃO: cancela temporizador pendente ao trocar de modo
      registrarEvento();
    }
    request->send(200, "application/json", "{\"mensagem\":\"MODO MANUAL\"}");
  });

  // Ativa modo automático
  server.on("/modo/auto", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!modoAutomatico) {
      modoAutomatico = true;
      temporizadorAtivo = false; // CORREÇÃO: começa "limpo" ao voltar pro automático
      registrarEvento();
    }
    request->send(200, "application/json", "{\"mensagem\":\"MODO AUTOMATICO\"}");
  });

  // CORREÇÃO: rota nova — antes não existia NENHUMA forma de configurar
  // correnteLimite/tempoDesligamento pela interface web. Ex: /config?limite=1.5&tempo=30
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("limite")) {
      float novoLimite = request->getParam("limite")->value().toFloat();
      if (novoLimite > 0) correnteLimite = novoLimite;
    }
    if (request->hasParam("tempo")) {
      int novoTempo = request->getParam("tempo")->value().toInt();
      if (novoTempo > 0) tempoDesligamento = novoTempo;
    }

    DynamicJsonDocument doc(128);
    doc["limite_corrente"] = correnteLimite;
    doc["tempo_desligamento_min"] = tempoDesligamento;
    String resposta;
    serializeJson(doc, resposta);
    request->send(200, "application/json", resposta);
  });

  // CORREÇÃO: rota nova — recebe a hora real (epoch em ms) do navegador que
  // conectou e calcula o offset entre ela e o millis() do ESP. É isso que
  // resolve os eventos aparecendo com data 31/12/1969 no histórico.
  server.on("/setclock", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("agora")) {
      double epocaRecebida = atof(request->getParam("agora")->value().c_str());
      if (epocaRecebida > 0) {
        relogioOffsetMs = epocaRecebida - (double)millis();
      }
    }
    request->send(200, "application/json", "{\"ok\":true}");
  });

  // Retorna o histórico completo
  server.on("/historico", HTTP_GET, [](AsyncWebServerRequest *request){

    DynamicJsonDocument doc(2048);
    JsonArray array = doc.to<JsonArray>();

    // CORREÇÃO: percorre apenas os eventos realmente registrados, do mais antigo
    // pro mais recente, em vez de despejar o array cru (que tinha posições zeradas).
    int qtd = totalEventosRegistrados;
    int inicio = (qtd < MAX_EVENTOS) ? 0 : indiceHistorico;

    for (int i = 0; i < qtd; i++) {
      int idx = (inicio + i) % MAX_EVENTOS;
      JsonObject ev = array.createNestedObject();
      // CORREÇÃO: converte o millis() bruto pra data/hora real usando o offset
      // sincronizado (se ainda não sincronizou, cai no millis() cru como antes).
      double tempoReal = (relogioOffsetMs < 0)
                            ? (double)historico[idx].tempo
                            : (relogioOffsetMs + (double)historico[idx].tempo);
      ev["tempo"] = tempoReal;
      ev["rele"] = historico[idx].estadoRele;
      ev["auto"] = historico[idx].modoAuto;
      ev["corrente"] = historico[idx].correnteEvento;
    }

    String resposta;
    serializeJson(doc, resposta);
    request->send(200, "application/json", resposta);
  });

  server.begin();
  Serial1.println("Servidor iniciado.");
}


/* ======================== LOOP (EXECUTA SEMPRE) ======================== */

void loop() {

  /* -------- LEITURA DO PZEM -------- */

  tensao = pzem.voltage();
  corrente = pzem.current();
  potencia = pzem.power();
  energia = pzem.energy();
  frequencia = pzem.frequency();
  fatorPotencia = pzem.pf();

  /* -------- LÓGICA DO MODO AUTOMÁTICO -------- */

  if (modoAutomatico && !isnan(corrente)) {

    if (corrente > correnteLimite) {

      if (!releLigado) {
        ligarRele();
        temporizadorAtivo = false;
      } else {
        // CORREÇÃO: se a carga voltou enquanto o temporizador de desligamento
        // estava contando, cancela a contagem (senão ele desligava mesmo com carga).
        temporizadorAtivo = false;
      }

    } else {

      if (releLigado && !temporizadorAtivo) {
        inicioTemporizador = millis();
        temporizadorAtivo = true;
      }

      if (temporizadorAtivo) {
        if (millis() - inicioTemporizador >= (unsigned long)tempoDesligamento * 60000UL) {
          desligarRele();
          temporizadorAtivo = false;
        }
      }
    }
  }

  delay(1000);
}
