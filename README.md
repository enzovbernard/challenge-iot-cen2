# Sistema IoT de Monitoramento de Saúde e Bem-Estar Pet 

Este projeto consiste em um ecossistema de Internet das Coisas (IoT) voltado para monitorar continuamente o comportamento de repouso de animais de estimação e as variáveis ambientais do local de descanso, gerando diagnósticos preventivos calculados diretamente no dispositivo.

---

## Tecnologias Utilizadas
* **Hardware:** ESP32, Sensor de Temperatura/Umidade DHT22, Sensor de Presença PIR, Buzzer, Push Buttons e LEDs.
* **Código-Fonte:** C++ (Framework Arduino)
* **Plataforma de Simulação:** [Wokwi](https://wokwi.com/)
* **Plataforma Cloud IoT:** [Thinger.io](https://thinger.io/)

---

## Como o Projeto Funciona (Lógica e Algoritmo)

O sistema opera através de um algoritmo de processamento local executado diretamente no ESP32. Para viabilizar testes rápidos, o tempo foi escalonado: **1 segundo real equivale a 1 hora simulada do pet** (1 ciclo diário completo = 24 segundos reais).

### 1. Controle de Ciclo e Captura de Dados
* **Início do Dia:** O ciclo de 24 segundos (24 horas simuladas) só é iniciado quando o responsável pressiona o Botão de Inicialização. Isso garante total controle para o início dos testes.
* **Monitoramento Ambiental:** A cada 5 segundos, o ESP32 lê a temperatura e a umidade do ar através do sensor DHT22.
* **Monitoramento de Sono:** O sistema detecta quando o pet deita ou levanta através de duas fontes: o sensor **PIR** (automação por presença) ou os **Botões Físicos** (acionamento manual pelo responsável).
* **Interação Remota (Atuador):** Através do painel na nuvem, o usuário pode acionar um interruptor que ativa o Buzzer localmente no ESP32, servindo como um alarme sonoro para acordar o animal à distância.

### 2. Algoritmo de Cálculo do Score (Processamento Local)
A cada ciclo de 24 segundos (1 dia simulado), o ESP32 analisa os dados acumulados e calcula uma pontuação de saúde chamada **Score de Descanso** (que inicia em 100) aplicando as seguintes penalidades:
* **Tempo de Sono Insuficiente:** Se o pet dormiu menos de 6 horas simuladas no ciclo, perde 40 pontos. Se dormiu entre 6 e 12 horas, perde 15 pontos.
* **Tempo de Sono Excessivo:** Se dormiu mais de 18 horas simuladas, perde 20 pontos.
* **Sono Fragmentado (Interrompido):** Considerando o padrão biológico dos pets, que naturalmente acordam várias vezes, o sistema foi calibrado para ser tolerante. O score só sofre penalidade de 10 pontos se o pet tiver entre 7 e 11 interrupções, e perde 20 pontos caso atinja 12 ou mais interrupções no mesmo ciclo (sinal de forte agitação, coceira excessiva ou estresse).
* **Estresse Térmico:** Se a temperatura ambiente ultrapassar os 30.0°C, perde 15 pontos. Se cair abaixo de 15.0°C, perde 10 pontos.

### 3. Classificação e Diagnóstico
Com base no Score final calculado localmente, o ESP32 classifica a saúde do repouso do pet em três níveis e gera diagnósticos proativos contextuais:
* **Score acima de 75:** Status NORMAL | O sistema valida o sono de qualidade e aponta se o ambiente se manteve em condições ideais ou se registrou extremos de temperatura (muito quente/frio).
* **Score entre 50 e 75:** Status ATENCAO | O algoritmo detalha o causador da instabilidade, alertando explicitamente sobre interrupções excessivas (sono fragmentado), tempo de repouso abaixo do recomendado, sono em excesso ou variações climáticas prejudiciais.
* **Score abaixo de 50:** Status CRITICO | Dispara o LED Vermelho de Alerta e gera avisos claros sobre cenários graves, como a combinação de privação de sono com alta agitação ou em noites onde o animal passa o ciclo inteiro quase sem conseguir dormir.

---

## Integração com a Nuvem (Thinger.io)

Ao final de cada ciclo, o ESP32 transmite os resultados diretamente para o painel do Thinger.io através de 5 canais de dados independentes configurados via transmissão por dispositivo:

1. `analise_ia` (Text) - Recomendação proativa interpretada pelo algoritmo local.
2. `score_descanso` (Gauge) - Nota final de 0 a 100 da qualidade do repouso.
3. `temperatura_pet` (Gauge) - Temperatura do ambiente em tempo real (°C).
4. `umidade_pet` (Gauge) - Umidade relativa do ar (%).
5. `total_sono` (Text) - Acumulado de horas que o pet dormiu no ciclo.
6. `interrupcoes` (Text) - Quantidade de vezes que o animal acordou.
7. `media_sono` (Text) - Média de duração das sessões de sono.
8. `alarme_pet` (On/Off Switch) - Canal de controle para acionar o alarme (Buzzer) à distância.

---

## Como Executar e Testar 

1. Abra a simulação do circuito no Wokwi através do link público: https://wokwi.com/projects/464233379928396801
2. Clique no botão de **Play** para iniciar a execução e aguarde a conexão Wi-Fi ser estabelecida.
3. Pressione o **Botão Azul (Iniciar)** no circuito para dar início ao monitoramento do dia simulado.
4. **Teste do Alarme:** No painel do Thinger.io, ligue o interruptor "Acordar Pet" e verifique o acionamento sonoro imediato do Buzzer no Wokwi.
5. **Teste de Repouso:** Interaja com o sensor PIR ou botões de deitar/levantar para simular a rotina do animal. Após 24 segundos do início do ciclo, o dia se encerra, envia os dados para os gráficos da nuvem e exibe o relatório detalhado no Terminal Serial, aguardando um novo clique no Botão Azul para o próximo dia.

---
## Autoria e Identificação
* **Integrantes:**
	* Caio Kenzo Tayra - RM562979
	* Enzo Vieira Bernardini - RM563000
	* Nícolas Mota Cândido - RM561857
	* Natan Freitas De Moraes - RM564992
* **Turma:** [2TDSPI]
* **Instituição:** FIAP - 2026
