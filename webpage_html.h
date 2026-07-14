#ifndef WEBPAGE_HTML_H
#define WEBPAGE_HTML_H

// CORRECAO: o HTML precisa ficar em um arquivo .h separado (fora do .ino).
// O Arduino IDE gera prototipos de funcao automaticamente varrendo o .ino
// como texto puro (via ctags), sem entender que este trecho e uma string.
// Ele encontrava "function nome() {" do JavaScript embutido e tentava tratar
// como uma funcao C++, causando erros como "'function' does not name a type".
// Arquivos .h/.cpp nao passam por esse scanner, entao o problema desaparece.

/* ======================== PAGINA HTML (EMBUTIDA NO FIRMWARE) ======================== */

// CORRECAO: a pagina HTML fica gravada na flash (PROGMEM) e e servida na rota "/".
// Sem isso, o ESPAsyncWebServer nao tinha NENHUMA rota "/" registrada, entao ao
// acessar http://192.168.4.1 o navegador nao recebia resposta nenhuma.
const char PAGINA_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>NEOENERGIA - Monitoramento</title>

<style>
:root {
  --azul: #4f9fe6;
  --verde: #3e8f63;
  --laranja: #f4a261;
  --cinza: #b0b0b0;
  --vermelho: #e63946;
  --escuro: #2c3e50;
}

* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

body {
  font-family: 'Segoe UI', Arial, sans-serif;
  background: linear-gradient(135deg, var(--laranja) 0%, #e76f51 100%);
  min-height: 100vh;
}

header {
  background: linear-gradient(135deg, var(--azul) 0%, #2c6faa 100%);
  color: white;
  padding: 20px 14px;
  text-align: center;
  border-bottom: 3px solid var(--escuro);
  box-shadow: 0 4px 6px rgba(0,0,0,0.1);
}

header h1 {
  margin: 0;
  font-size: 2em;
  font-weight: bold;
  text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
  letter-spacing: 2px;
}

header h2 {
  margin: 8px 0 0;
  font-size: 0.9em;
  font-weight: normal;
  opacity: 0.9;
}

.container {
  max-width: 600px;
  margin: auto;
  padding: 20px;
}

.card {
  background: white;
  border-radius: 15px;
  padding: 20px;
  margin-top: 20px;
  border: 2px solid var(--escuro);
  box-shadow: 0 8px 16px rgba(0,0,0,0.1);
  transition: transform 0.3s ease;
}

.card:hover {
  transform: translateY(-2px);
}

.status-badge {
  display: inline-block;
  padding: 8px 16px;
  border-radius: 20px;
  font-weight: bold;
  font-size: 0.9em;
  margin: 10px 0;
}

.status-badge.success {
  background: #d4edda;
  color: #155724;
  border: 2px solid #c3e6cb;
}

.status-badge.danger {
  background: #f8d7da;
  color: #721c24;
  border: 2px solid #f5c6cb;
}

.status-badge.warning {
  background: #fff3cd;
  color: #856404;
  border: 2px solid #ffeaa7;
}

button {
  width: 100%;
  padding: 14px;
  margin-top: 10px;
  border: 2px solid var(--escuro);
  border-radius: 10px;
  font-size: 1em;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.3s ease;
  background: var(--verde);
  color: white;
  text-transform: uppercase;
  letter-spacing: 1px;
}

button:hover:not(:disabled) {
  transform: translateY(-2px);
  box-shadow: 0 4px 8px rgba(0,0,0,0.2);
}

button:active:not(:disabled) {
  transform: translateY(0);
}

button:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

button.gray {
  background: var(--cinza);
}

button.danger {
  background: var(--vermelho);
}

button.active {
  font-weight: bold;
  outline: 3px solid var(--escuro);
  transform: scale(1.05);
  box-shadow: 0 6px 12px rgba(0,0,0,0.3);
}

.dual {
  display: flex;
  gap: 10px;
}

.dual button {
  width: 50%;
}

input {
  width: 100%;
  padding: 12px;
  margin: 8px 0;
  display: block;
  border: 2px solid var(--escuro);
  border-radius: 8px;
  font-size: 1em;
  transition: border-color 0.3s;
}

input:focus {
  outline: none;
  border-color: var(--azul);
  box-shadow: 0 0 0 3px rgba(79, 159, 230, 0.3);
}

input:invalid {
  border-color: var(--vermelho);
}

.center {
  text-align: center;
}

#dados {
  text-align: left;
  background: #f8f9fa;
  padding: 15px;
  border-radius: 10px;
  border: 1px solid #dee2e6;
}

#dados div {
  padding: 8px 0;
  border-bottom: 1px solid #dee2e6;
  display: flex;
  justify-content: space-between;
}

#dados div:last-child {
  border-bottom: none;
}

.alert {
  padding: 12px;
  border-radius: 8px;
  margin: 10px 0;
  font-weight: bold;
  text-align: center;
}

.alert-error {
  background: #f8d7da;
  color: #721c24;
  border: 2px solid #f5c6cb;
}

.alert-success {
  background: #d4edda;
  color: #155724;
  border: 2px solid #c3e6cb;
}

.alert-info {
  background: #d1ecf1;
  color: #0c5460;
  border: 2px solid #bee5eb;
}

.loading {
  display: inline-block;
  width: 20px;
  height: 20px;
  border: 3px solid #f3f3f3;
  border-top: 3px solid var(--azul);
  border-radius: 50%;
  animation: spin 1s linear infinite;
  margin-left: 10px;
}

@keyframes spin {
  0% { transform: rotate(0deg); }
  100% { transform: rotate(360deg); }
}

footer {
  text-align: center;
  margin-top: 40px;
  padding: 20px;
  font-size: 0.9em;
  color: white;
  text-shadow: 1px 1px 2px rgba(0,0,0,0.5);
}

#clock {
  font-size: 1.2em;
  font-weight: bold;
}

@media (max-width: 480px) {
  .container {
    padding: 10px;
  }
  
  .card {
    padding: 15px;
    margin-top: 15px;
  }
  
  button {
    padding: 12px;
    font-size: 0.9em;
  }
  
  header h1 {
    font-size: 1.5em;
  }
}
</style>
</head>

<body>

<header>
  <h1>⚡ NEOENERGIA</h1>
  <h2>CTT – Controle de Temperatura para Transformadores</h2>
  <div id="connectionStatus" class="status-badge warning">Conectando...</div>
</header>

<div class="container">

<!-- MENU PRINCIPAL -->
<div class="card" id="menu">
  <div id="statusGeral" style="text-align: center; margin-bottom: 15px;"></div>
  
  <div class="dual">
    <button id="btnOn" disabled>🔌 Relé<br>Ativado</button>
    <button id="btnOff" disabled>⭕ Relé<br>Desativado</button>
  </div>

  <div class="dual">
    <button id="btnManual">🔧 Modo<br>Manual</button>
    <button id="btnAuto">🤖 Modo<br>Automático</button>
  </div>

  <button onclick="mostrarTela('mon')">📊 Monitoramento</button>
  <button onclick="mostrarTela('cfg')">⚙️ Configuração</button>
  <button onclick="mostrarTela('tmp')">🌡️ Temperatura</button>
  <button onclick="mostrarTela('his')">📋 Histórico</button>
</div>

<!-- MONITORAMENTO -->
<div class="card" id="mon" style="display:none">
  <h3 class="center">📊 Dados em Tempo Real</h3>
  <div id="dados">
    <div><span>⚡ Tensão:</span> <strong><span id="v">---</span> V</strong></div>
    <div><span>🔌 Corrente:</span> <strong><span id="i">---</span> A</strong></div>
    <div><span>💡 Potência:</span> <strong><span id="p">---</span> W</strong></div>
    <div><span>📶 Frequência:</span> <strong><span id="f">---</span> Hz</strong></div>
    <div><span>📐 Fator Potência:</span> <strong><span id="fp">---</span></strong></div>
    <div><span>⚙️ Estado Relé:</span> <strong><span id="statusRele">---</span></strong></div>
    <div><span>🔄 Modo:</span> <strong><span id="statusModo">---</span></strong></div>
  </div>
  <div id="erroSensor" class="alert alert-error" style="display:none;">
    ⚠️ Erro no sensor! Verifique a conexão do PZEM.
  </div>
  <button onclick="voltarMenu()">⬅️ Voltar</button>
</div>

<!-- CONFIGURAÇÃO -->
<div class="card" id="cfg" style="display:none">
  <h3 class="center">⚙️ Configuração do Sistema</h3>
  
  <div class="center"><b>Corrente Limite para Acionamento (A)</b></div>
  <input type="number" id="correnteLimite" min="0.1" max="100" step="0.1" placeholder="Ex: 1.0" required>
  
  <div class="center"><b>Tempo para Desligamento (minutos)</b></div>
  <input type="number" id="tempoDesligamento" min="1" max="1440" step="1" placeholder="Ex: 40" required>
  
  <div id="configStatus"></div>
  
  <button onclick="salvarConfiguracao()">💾 Salvar Configuração</button>
  <button onclick="voltarMenu()">⬅️ Voltar</button>
</div>

<!-- TEMPERATURA -->
<div class="card" id="tmp" style="display:none">
  <h3 class="center">🌡️ Monitoramento Térmico</h3>
  
  <div class="center"><b>Temperatura do Óleo (°C)</b></div>
  <input type="number" id="tempOleo" min="-20" max="200" step="0.1" placeholder="Temperatura do óleo">
  
  <div class="center" style="margin-top: 20px;">
    <b>Temperatura Estimada do Enrolamento</b>
    <p style="font-size: 2em; margin: 10px 0; color: var(--vermelho);" id="tempEnrol">--- °C</p>
    <small style="color: #666;">Estimativa baseada na corrente de carga (IEC 60076-7)</small>
  </div>
  
  <div id="alertaTemperatura" style="display:none;"></div>
  
  <button onclick="voltarMenu()">⬅️ Voltar</button>
</div>

<!-- HISTÓRICO -->
<div class="card" id="his" style="display:none">
  <h3 class="center">📋 Histórico de Eventos</h3>
  <div id="histContainer" style="max-height: 400px; overflow-y: auto; background: #f8f9fa; padding: 10px; border-radius: 8px;">
    <div id="hist">Carregando histórico...</div>
  </div>
  <button onclick="limparHistorico()" class="danger">🗑️ Limpar Histórico</button>
  <button onclick="voltarMenu()">⬅️ Voltar</button>
</div>

<footer>
  <div id="clock"></div>
  <div style="margin-top: 20px;">Desenvolvido por: <strong>Blumenal</strong> em colaboração com <strong>Devid Benner</strong></div>
  <div style="margin-top: 8px;">
    <a href="https://github.com/blumenal/CTT_Energia" target="_blank" rel="noopener noreferrer" style="color: white; text-decoration: underline;">
      📦 github.com/blumenal/CTT_Energia
    </a>
  </div>
  <div style="margin-top: 10px; font-size: 0.8em;">Versão 2.0 · Licença MIT</div>
</footer>

</div>

<script>
/* ================= CONFIGURAÇÃO ================= */
const ESP = "http://192.168.4.1";
const INTERVALO_ATUALIZACAO = 2000; // 2 segundos
const INTERVALO_RELOGIO = 1000; // 1 segundo

/* ================= ESTADO DO SISTEMA ================= */
const estado = {
  modo: 'manual',
  rele: false,
  conectado: false,
  erroSensor: false,
  config: {
    correnteLimite: 1.0,
    tempoDesligamento: 40
  }
};

/* ================= ELEMENTOS DOM ================= */
const elementos = {
  menu: document.getElementById('menu'),
  mon: document.getElementById('mon'),
  cfg: document.getElementById('cfg'),
  tmp: document.getElementById('tmp'),
  his: document.getElementById('his'),
  
  btnOn: document.getElementById('btnOn'),
  btnOff: document.getElementById('btnOff'),
  btnManual: document.getElementById('btnManual'),
  btnAuto: document.getElementById('btnAuto'),
  
  v: document.getElementById('v'),
  i: document.getElementById('i'),
  p: document.getElementById('p'),
  f: document.getElementById('f'),
  fp: document.getElementById('fp'),
  statusRele: document.getElementById('statusRele'),
  statusModo: document.getElementById('statusModo'),
  
  correnteLimite: document.getElementById('correnteLimite'),
  tempoDesligamento: document.getElementById('tempoDesligamento'),
  configStatus: document.getElementById('configStatus'),
  
  tempOleo: document.getElementById('tempOleo'),
  tempEnrol: document.getElementById('tempEnrol'),
  alertaTemperatura: document.getElementById('alertaTemperatura'),
  
  hist: document.getElementById('hist'),
  erroSensor: document.getElementById('erroSensor'),
  connectionStatus: document.getElementById('connectionStatus'),
  statusGeral: document.getElementById('statusGeral'),
  clock: document.getElementById('clock')
};

/* ================= UTILITÁRIOS ================= */
function formatarData() {
  const d = new Date();
  return `${d.toLocaleDateString('pt-BR')} ${d.toLocaleTimeString('pt-BR')}`;
}

function mostrarNotificacao(mensagem, tipo = 'info') {
  const statusGeral = elementos.statusGeral;
  statusGeral.className = `alert alert-${tipo}`;
  statusGeral.textContent = mensagem;
  statusGeral.style.display = 'block';
  
  setTimeout(() => {
    statusGeral.style.display = 'none';
  }, 3000);
}

/* ================= GERENCIAMENTO DE TELAS ================= */
function mostrarTela(id) {
  ['menu', 'mon', 'cfg', 'tmp', 'his'].forEach(tela => {
    document.getElementById(tela).style.display = 'none';
  });
  document.getElementById(id).style.display = 'block';
  
  if (id === 'cfg') {
    carregarConfiguracaoAtual();
  } else if (id === 'his') {
    carregarHistorico();
  }
}

function voltarMenu() {
  mostrarTela('menu');
}

/* ================= SINCRONIZAÇÃO DE RELÓGIO ================= */
// CORREÇÃO: o ESP8266 não tem RTC (relógio de tempo real) e, rodando só como
// Access Point (sem internet), não consegue usar NTP. Por isso os eventos do
// histórico apareciam com data 31/12/1969 (o ESP só sabia "milissegundos desde
// que ligou", não a data real). Aqui mandamos a hora atual do celular/PC
// (que já sabe a hora certa) pro ESP guardar a diferença e calcular a data
// real de cada evento a partir dali.
async function sincronizarRelogio() {
  try {
    await requisicaoESP(`/setclock?agora=${Date.now()}`);
  } catch (erro) {
    console.error('Erro ao sincronizar relógio:', erro);
  }
}

/* ================= COMUNICAÇÃO COM ESP ================= */
async function requisicaoESP(endpoint, metodo = 'GET', body = null) {
  try {
    const opcoes = {
      method: metodo,
      headers: {
        'Content-Type': 'application/json',
        'Authorization': 'Basic ' + btoa('admin:admin123')
      }
    };
    
    if (body) {
      opcoes.body = JSON.stringify(body);
    }
    
    const resposta = await fetch(`${ESP}${endpoint}`, opcoes);
    
    if (!resposta.ok) {
      throw new Error(`HTTP ${resposta.status}: ${resposta.statusText}`);
    }
    
    return await resposta.json();
  } catch (erro) {
    console.error(`Erro na requisição ${endpoint}:`, erro);
    throw erro;
  }
}

/* ================= ATUALIZAÇÃO DE STATUS ================= */
async function atualizarStatus() {
  try {
    const dados = await requisicaoESP('/status');
    
    estado.conectado = true;
    estado.erroSensor = dados.erroSensor || false;
    estado.modo = dados.automatico ? 'auto' : 'manual';
    estado.rele = dados.rele;
    
    // Atualiza dados de monitoramento
    elementos.v.textContent = dados.tensao ? dados.tensao.toFixed(1) : '---';
    elementos.i.textContent = dados.corrente ? dados.corrente.toFixed(2) : '---';
    elementos.p.textContent = dados.potencia ? dados.potencia.toFixed(1) : '---';
    elementos.f.textContent = dados.frequencia ? dados.frequencia.toFixed(1) : '---';
    elementos.fp.textContent = dados.fp ? dados.fp.toFixed(2) : '---';
    
    // Atualiza status do relé e modo
    elementos.statusRele.textContent = estado.rele ? 'LIGADO' : 'DESLIGADO';
    elementos.statusModo.textContent = estado.modo === 'auto' ? 'AUTOMÁTICO' : 'MANUAL';
    
    // Atualiza indicador de erro do sensor
    elementos.erroSensor.style.display = estado.erroSensor ? 'block' : 'none';
    
    // Atualiza estado da conexão
    elementos.connectionStatus.textContent = '✅ Conectado';
    elementos.connectionStatus.className = 'status-badge success';
    
    // Atualiza visual dos botões
    atualizarVisual();
    
    // Atualiza temperatura se estiver na tela
    if (elementos.tmp.style.display !== 'none') {
      calcularTemperatura(dados.corrente);
    }
    
  } catch (erro) {
    estado.conectado = false;
    elementos.connectionStatus.textContent = '❌ Desconectado';
    elementos.connectionStatus.className = 'status-badge danger';
    elementos.erroSensor.style.display = 'block';
    elementos.erroSensor.textContent = '⚠️ Erro de conexão com o ESP8266. Verifique a rede.';
  }
}

/* ================= CONTROLE DO RELÉ E MODO ================= */
async function enviarComando(endpoint, mensagemSucesso) {
  try {
    await requisicaoESP(endpoint);
    mostrarNotificacao(mensagemSucesso, 'success');
    await atualizarStatus();
    adicionarAoHistorico(mensagemSucesso);
  } catch (erro) {
    mostrarNotificacao(`Erro: ${erro.message}`, 'error');
  }
}

elementos.btnOn.addEventListener('click', () => {
  if (estado.modo === 'auto') {
    mostrarNotificacao('Desative o modo automático primeiro', 'error');
    return;
  }
  enviarComando('/rele/on', 'Relé ligado com sucesso');
});

elementos.btnOff.addEventListener('click', () => {
  if (estado.modo === 'auto') {
    mostrarNotificacao('Desative o modo automático primeiro', 'error');
    return;
  }
  enviarComando('/rele/off', 'Relé desligado com sucesso');
});

elementos.btnManual.addEventListener('click', () => {
  enviarComando('/modo/manual', 'Modo manual ativado');
});

elementos.btnAuto.addEventListener('click', () => {
  enviarComando('/modo/auto', 'Modo automático ativado');
});

/* ================= CONFIGURAÇÃO ================= */
async function carregarConfiguracaoAtual() {
  try {
    const dados = await requisicaoESP('/status');
    elementos.correnteLimite.value = estado.config.correnteLimite;
    elementos.tempoDesligamento.value = estado.config.tempoDesligamento;
  } catch (erro) {
    console.error('Erro ao carregar configuração:', erro);
  }
}

async function salvarConfiguracao() {
  const corrente = parseFloat(elementos.correnteLimite.value);
  const tempo = parseInt(elementos.tempoDesligamento.value);
  
  // Validação
  if (isNaN(corrente) || corrente < 0.1 || corrente > 100) {
    elementos.configStatus.innerHTML = '<div class="alert alert-error">Corrente inválida (0.1 - 100 A)</div>';
    return;
  }
  
  if (isNaN(tempo) || tempo < 1 || tempo > 1440) {
    elementos.configStatus.innerHTML = '<div class="alert alert-error">Tempo inválido (1 - 1440 min)</div>';
    return;
  }
  
  try {
    await requisicaoESP(`/config?limite=${corrente}&tempo=${tempo}`);
    
    estado.config.correnteLimite = corrente;
    estado.config.tempoDesligamento = tempo;
    
    elementos.configStatus.innerHTML = '<div class="alert alert-success">✅ Configuração salva com sucesso!</div>';
    
    setTimeout(() => {
      elementos.configStatus.innerHTML = '';
    }, 3000);
    
  } catch (erro) {
    elementos.configStatus.innerHTML = `<div class="alert alert-error">❌ Erro ao salvar: ${erro.message}</div>`;
  }
}

/* ================= CÁLCULO DE TEMPERATURA ================= */
function calcularTemperatura(corrente = 0) {
  const tempOleo = parseFloat(elementos.tempOleo.value);
  
  if (isNaN(tempOleo)) {
    elementos.tempEnrol.textContent = '--- °C';
    return;
  }
  
  // Validação da temperatura do óleo
  if (tempOleo < -20 || tempOleo > 200) {
    elementos.alertaTemperatura.style.display = 'block';
    elementos.alertaTemperatura.className = 'alert alert-error';
    elementos.alertaTemperatura.textContent = '⚠️ Temperatura do óleo fora dos limites (-20°C a 200°C)';
    return;
  }
  
  // Cálculo baseado na norma IEC 60076-7 (estimativa simplificada)
  // ΔT = H * (I/In)^1.6, onde H é o gradiente térmico nominal (tipicamente 10-15°C)
  const correnteNominal = 100; // Ajustar conforme especificação do transformador
  const gradienteTermico = 12; // °C (valor típico para transformadores a óleo)
  
  let elevacaoTemperatura = 0;
  
  if (corrente > 0 && correnteNominal > 0) {
    const razaoCorrente = Math.min(corrente / correnteNominal, 2.0); // Limita a 200%
    elevacaoTemperatura = gradienteTermico * Math.pow(razaoCorrente, 1.6);
  }
  
  const tempEnrolamento = tempOleo + elevacaoTemperatura;
  
  // Atualiza display
  elementos.tempEnrol.textContent = tempEnrolamento.toFixed(1) + ' °C';
  
  // Alertas de temperatura
  elementos.alertaTemperatura.style.display = 'block';
  
  if (tempEnrolamento > 140) {
    elementos.alertaTemperatura.className = 'alert alert-error';
    elementos.alertaTemperatura.textContent = '🔴 CRÍTICO: Temperatura do enrolamento excede 140°C! Desligue imediatamente!';
  } else if (tempEnrolamento > 120) {
    elementos.alertaTemperatura.className = 'alert alert-error';
    elementos.alertaTemperatura.textContent = '🟠 ALERTA: Temperatura do enrolamento acima de 120°C!';
  } else if (tempEnrolamento > 100) {
    elementos.alertaTemperatura.className = 'alert alert-info';
    elementos.alertaTemperatura.textContent = '🟡 ATENÇÃO: Temperatura do enrolamento elevada (acima de 100°C)';
  } else {
    elementos.alertaTemperatura.className = 'alert alert-success';
    elementos.alertaTemperatura.textContent = '✅ Temperatura do enrolamento dentro dos limites normais';
  }
}

elementos.tempOleo.addEventListener('input', () => calcularTemperatura(
  parseFloat(elementos.i.textContent) || 0
));

/* ================= HISTÓRICO ================= */
async function carregarHistorico() {
  try {
    const dados = await requisicaoESP('/historico');
    
    if (!dados || dados.length === 0) {
      elementos.hist.innerHTML = 'Nenhum evento registrado';
      return;
    }
    
    let html = '';
    dados.forEach(evento => {
      const data = new Date(evento.tempo);
      const dataFormatada = data.toLocaleString('pt-BR');
      const estadoRele = evento.rele ? 'LIGADO' : 'DESLIGADO';
      const modo = evento.auto ? 'AUTOMÁTICO' : 'MANUAL';
      
      html += `
        <div style="padding: 8px; border-bottom: 1px solid #dee2e6;">
          <strong>${dataFormatada}</strong><br>
          Relé: ${estadoRele} | Modo: ${modo}<br>
          Corrente: ${evento.corrente.toFixed(2)} A
        </div>
      `;
    });
    
    elementos.hist.innerHTML = html;
  } catch (erro) {
    elementos.hist.innerHTML = 'Erro ao carregar histórico';
    console.error('Erro ao carregar histórico:', erro);
  }
}

function adicionarAoHistorico(mensagem) {
  const data = formatarData();
  const entrada = `${data} - ${mensagem}`;
  
  // Também salva no localStorage como backup
  const historicoLocal = JSON.parse(localStorage.getItem('historicoLocal') || '[]');
  historicoLocal.unshift({ data, mensagem, timestamp: Date.now() });
  
  // Mantém apenas os últimos 100 registros
  if (historicoLocal.length > 100) {
    historicoLocal.splice(100);
  }
  
  localStorage.setItem('historicoLocal', JSON.stringify(historicoLocal));
}

function limparHistorico() {
  if (confirm('Tem certeza que deseja limpar o histórico local?')) {
    localStorage.removeItem('historicoLocal');
    elementos.hist.innerHTML = 'Nenhum evento registrado';
    mostrarNotificacao('Histórico limpo com sucesso', 'success');
  }
}

/* ================= ATUALIZAÇÃO VISUAL ================= */
function atualizarVisual() {
  // Modo
  elementos.btnManual.className = estado.modo === 'manual' ? 'active' : '';
  elementos.btnAuto.className = estado.modo === 'auto' ? 'active' : '';
  
  // Relé
  if (estado.modo === 'auto') {
    elementos.btnOn.disabled = true;
    elementos.btnOff.disabled = true;
    elementos.btnOn.className = 'gray';
    elementos.btnOff.className = 'gray';
  } else {
    elementos.btnOn.disabled = false;
    elementos.btnOff.disabled = false;
    elementos.btnOn.className = estado.rele ? 'active' : '';
    elementos.btnOff.className = !estado.rele ? 'active' : '';
  }
  
  // Status geral no menu
  const statusRele = estado.rele ? 'LIGADO' : 'DESLIGADO';
  const statusModo = estado.modo === 'auto' ? 'AUTOMÁTICO' : 'MANUAL';
  elementos.statusGeral.innerHTML = `
    <strong>Relé: ${statusRele}</strong> | 
    <strong>Modo: ${statusModo}</strong>
  `;
}

/* ================= RELÓGIO ================= */
function atualizarRelogio() {
  const d = new Date();
  elementos.clock.innerHTML = `
    ${d.toLocaleTimeString('pt-BR')}<br>
    <small>${d.toLocaleDateString('pt-BR', { weekday: 'long', year: 'numeric', month: 'long', day: 'numeric' })}</small>
  `;
}

/* ================= INICIALIZAÇÃO ================= */
function inicializar() {
  // Carrega configuração do localStorage como fallback
  const configLocal = JSON.parse(localStorage.getItem('configLocal'));
  if (configLocal) {
    estado.config = configLocal;
  }
  
  // CORREÇÃO: sincroniza o relógio do ESP com a hora real do dispositivo assim
  // que a página conecta, para os eventos do histórico terem a data correta.
  sincronizarRelogio();

  // Primeira atualização
  atualizarStatus();
  atualizarRelogio();
  
  // Configura intervalos
  setInterval(atualizarStatus, INTERVALO_ATUALIZACAO);
  setInterval(atualizarRelogio, INTERVALO_RELOGIO);
  
  // Monitora visibilidade da página para otimizar atualizações
  document.addEventListener('visibilitychange', () => {
    if (document.hidden) {
      // Pausa atualizações quando a página não está visível
      clearInterval(window.atualizacaoInterval);
    } else {
      // Retoma atualizações
      atualizarStatus();
      window.atualizacaoInterval = setInterval(atualizarStatus, INTERVALO_ATUALIZACAO);
    }
  });
}

// Inicia a aplicação
document.addEventListener('DOMContentLoaded', inicializar);
</script>

</body>
</html>)rawliteral";

#endif
