# Sistema IoT de Monitoramento de Saúde e Bem-Estar Pet 

Este projeto consiste em um ecossistema de Internet das Coisas (IoT) voltado para monitorar continuamente o comportamento de repouso de animais de estimação e as variáveis ambientais do local de descanso, gerando diagnósticos preventivos calculados diretamente no dispositivo.

---

## Tecnologias Utilizadas
* **Hardware:** ESP32, Sensor de Temperatura/Umidade DHT22, Sensor de Presença PIR, Push Buttons e LEDs.
* **Firmware:** C++ (Framework Arduino)
* **Plataforma de Simulação:** [Wokwi](https://wokwi.com/)
* **Plataforma Cloud IoT:** [Thinger.io](https://thinger.io/)

---

## Como o Projeto Funciona (Lógica e Algoritmo)

O sistema opera através de um algoritmo de processamento local executado diretamente no ESP32. Para viabilizar testes rápidos, o tempo foi escalonado: **1 segundo real equivale a 1 hora simulada do pet** (1 ciclo diário completo = 24 segundos reais).

### 1. Captura de Dados e Eventos
* **Monitoramento Ambiental:** A cada 5 segundos, o ESP32 lê a temperatura e a umidade do ar através do sensor **DHT22**.
* **Monitoramento de Sono:** O sistema detecta quando o pet deita ou levanta através de duas fontes: o sensor **PIR** (automação por presença) ou os **Botões Físicos** (acionamento manual pelo responsável).

### 2. Algoritmo de Cálculo do Score (Processamento Local)
A cada ciclo de 24 segundos (1 dia simulado), o ESP32 analisa os dados acumulados e calcula uma pontuação de saúde chamada **Score de Descanso** (que inicia em 100) aplicando as seguintes penalidades:
* **Tempo de Sono Insuficiente:** Se o pet dormiu menos de 6 horas simuladas no ciclo, perde 40 pontos. Se dormiu entre 6 e 12 horas, perde 15 pontos.
* **Tempo de Sono Excessivo:** Se dormiu mais de 18 horas simuladas, perde 20 pontos.
* **Sono Fragmentado (Interrompido):** Se o pet acordou 5 ou mais vezes no ciclo, perde 25 pontos. Se acordou entre 3 e 4 vezes, perde 15 pontos.
* **Estresse Térmico:** Se a temperatura ambiente registrada pelo DHT22 ultrapassar os 30 graus, o score perde 10 pontos.

### 3. Classificação e Diagnóstico
Com base no Score final calculado, o ESP32 classifica a saúde do pet em três níveis:
* **Score acima de 75:** Status `NORMAL` | *"Excelente! O pet esta descansado."*
* **Score entre 50 e 75:** Status `ATENCAO` | *"Sono instavel. Monitore o ambiente e o pet."*
* **Score abaixo de 50:** Status `CRITICO` | *"Alerta! Pet privado de sono ou muito interrompido."* (Dispara o LED Vermelho de Alerta).

---

## Integração com a Nuvem (Thinger.io)

Ao final de cada ciclo, o ESP32 transmite os resultados diretamente para o painel do Thinger.io através de 5 canais de dados independentes configurados via transmissão por dispositivo:

1. `score_descanso` (Gauge) - Nota final de 0 a 100 da qualidade do repouso.
2. `temperatura_pet` (Gauge) - Temperatura em tempo real medida no ambiente (em graus).
3. `umidade_pet` (Gauge) - Umidade relativa do ar (%).
4. `status_clinico` (Text/Value) - Diagnóstico em texto (`NORMAL`, `ATENCAO` ou `CRITICO`).
5. `analise_ia` (Text/Value) - Recomendação proativa interpretada pelo algoritmo.

---

## Como Executar e Testar 

1. Abra a simulação do circuito no Wokwi através do link público: **https://wokwi.com/projects/464233379928396801**
2. Clique no botão de **Play** para iniciar a execução do firmware e a conexão Wi-Fi.
3. **Teste de Conforto:** Mantenha a temperatura estável no DHT22 e acione o sono uma única vez. Aguarde 24 segundos e veja o Thinger.io registrar Score 100 e Status NORMAL.
4. **Teste de Risco:** Suba o slider do DHT22 para mais de 30 graus e acione o sensor PIR ou os botões múltiplas vezes simulando agitação. O sistema derrubará o Score e atualizará o dashboard para ATENÇÃO/CRÍTICO na nuvem no próximo ciclo.

---
## Autoria e Identificação
* **Integrantes:** Caio Kenzo Tayra - RM562979 | Enzo Vieira Bernardini - RM563000 | Nícolas Mota Cândido - RM561857 | Natan Freitas De Moraes 
* **Turma:** [2TDSPI]
* **Instituição:** FIAP - 2026