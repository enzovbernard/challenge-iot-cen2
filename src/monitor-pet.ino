#include <WiFi.h>
#include <ThingerESP32.h>
#include "DHTesp.h"

#define USER_ID "enzobernardini"
#define DEVICE_ID "ESP32_Pet_Monitor"
#define DEVICE_CREDENTIAL ""

const char* ssid = "Wokwi-GUEST";
const char* password = "";

ThingerESP32 thing(USER_ID, DEVICE_ID, DEVICE_CREDENTIAL);

#define LED_ALERTA 26
#define LED_CONFIRMACAO 25
#define BUTTON_DEITAR 18
#define BUTTON_LEVANTAR 19
#define PIR_PIN 27
#define DHT_PIN 15
#define BUZZER_PIN 21
#define BUTTON_INICIAR 14

DHTesp dhtSensor;

float temperatura = 0;
float umidade = 0;
bool dormindo = false;
bool diaAtivo = false;
bool buzzerLigado = false;

bool ultimoBotaoDeitar = HIGH;
bool ultimoBotaoLevantar = HIGH;
bool ultimoBotaoIniciar = HIGH;
bool ultimoEstadoPIR = LOW;

unsigned long inicioSono = 0;
unsigned long ultimoClima = 0;
unsigned long inicioDia = 0;

unsigned long tempoSessao = 0;
unsigned long totalSono = 0;

int sessoesSono = 0;
int interrupcoes = 0;
float mediaSono = 0;
int scoreDescanso = 100;

String statusPet = "NORMAL";
String interpretacao = "Excelente! O pet esta descansado.";

void piscarConfirmacao() {
  digitalWrite(LED_CONFIRMACAO, HIGH);
  delay(120);
  digitalWrite(LED_CONFIRMACAO, LOW);
}

void piscarAlerta() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_ALERTA, HIGH);
    delay(200);
    digitalWrite(LED_ALERTA, LOW);
    delay(200);
  }
}

void iniciarSono(String origem) {
  if (!dormindo && diaAtivo) {
    dormindo = true;
    inicioSono = millis();
    sessoesSono++;
    piscarConfirmacao();
    
    Serial.print("O pet DEITOU (Origem: ");
    Serial.print(origem);
    Serial.println(")");
  }
}

void finalizarSono(String origem) {
  if (dormindo && diaAtivo) {
    dormindo = false;
    tempoSessao = (millis() - inicioSono) / 1000;
    totalSono += tempoSessao;
    interrupcoes++;
    piscarConfirmacao();

    Serial.print("O pet LEVANTOU (Origem: ");
    Serial.print(origem);
    Serial.print(") | Durou: ");
    Serial.print(tempoSessao);
    Serial.println(" horas simuladas");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_ALERTA, OUTPUT);
  pinMode(LED_CONFIRMACAO, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_DEITAR, INPUT_PULLUP);
  pinMode(BUTTON_LEVANTAR, INPUT_PULLUP);
  pinMode(BUTTON_INICIAR, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT);

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  Serial.println("            INICIALIZANDO MONITOR PET             ");
  Serial.println("       (Aperte o Botao Iniciar para comecar)      ");

  thing.add_wifi(ssid, password);

  thing["status_clinico"] >> [](pson& out){ out = statusPet.c_str(); };
  thing["analise_ia"] >> [](pson& out){ out = interpretacao.c_str(); };
  thing["score_descanso"] >> [](pson& out){ out = scoreDescanso; };
  thing["temperatura_pet"] >> [](pson& out){ out = temperatura; };
  thing["umidade_pet"] >> [](pson& out){ out = umidade; };
  thing["total_sono"] >> [](pson& out){ out = totalSono; };
  thing["interrupcoes"] >> [](pson& out){ out = interrupcoes; };
  thing["media_sono"] >> [](pson& out){ out = mediaSono; };

  thing["alarme_pet"] << [](pson& in){
    if(in.is_empty()){
      in = buzzerLigado;
    } else {
      buzzerLigado = in;
      if(buzzerLigado){
        digitalWrite(BUZZER_PIN, HIGH);
      } else {
        digitalWrite(BUZZER_PIN, LOW);
      }
    }
  };
}

void loop() {
  thing.handle();

  bool leituraIniciar = digitalRead(BUTTON_INICIAR);
  bool leituraDeitar = digitalRead(BUTTON_DEITAR);
  bool leituraLevantar = digitalRead(BUTTON_LEVANTAR);
  bool movimento = digitalRead(PIR_PIN);

  if (leituraIniciar == LOW && ultimoBotaoIniciar == HIGH) {
    if (!diaAtivo) {
      diaAtivo = true;
      inicioDia = millis();
      if (dormindo) {
        inicioSono = millis();
      }
      piscarConfirmacao();
      Serial.println("\nNOVO DIA INICIADO! Monitoramento ativo...");
    }
    delay(200);
  }
  ultimoBotaoIniciar = leituraIniciar;

  if (millis() - ultimoClima >= 5000) {
    TempAndHumidity data = dhtSensor.getTempAndHumidity();
    temperatura = data.temperature;
    umidade = data.humidity;
    ultimoClima = millis();
  }

  if (diaAtivo) {
    if (leituraDeitar == LOW && ultimoBotaoDeitar == HIGH) {
      iniciarSono("Botao");
    }

    if (leituraLevantar == LOW && ultimoBotaoLevantar == HIGH) {
      finalizarSono("Botao");
    }

    if (movimento == HIGH && ultimoEstadoPIR == LOW) {
      if (!dormindo) {
        iniciarSono("Sensor PIR");
      } else {
        finalizarSono("Sensor PIR");
      }
      delay(200); 
    }
    ultimoEstadoPIR = movimento;

    if (millis() - inicioDia >= 24000) {
      diaAtivo = false;

      if (dormindo) {
        unsigned long tempoParcial = (millis() - inicioSono) / 1000;
        totalSono += tempoParcial;
      }

      mediaSono = (sessoesSono > 0) ? (float)totalSono / sessoesSono : 0;

      if (totalSono == 0) {
        scoreDescanso = 0;
      } else if (totalSono < 5) {
        scoreDescanso = 30; 
      } else if (totalSono < 12) {
        scoreDescanso = 40 + (totalSono * 4); 
      } else if (totalSono <= 18) {
        scoreDescanso = 100; 
      } else {
        scoreDescanso = 100 - ((totalSono - 18) * 10); 
      }

      if (interrupcoes >= 12) scoreDescanso -= 20;
      else if (interrupcoes >= 7) scoreDescanso -= 10;

      if (temperatura > 30.0) scoreDescanso -= 15; 
      if (temperatura < 15.0) scoreDescanso -= 10; 

      if (scoreDescanso < 0) scoreDescanso = 0;
      if (scoreDescanso > 100) scoreDescanso = 100;

      if (scoreDescanso >= 75) {
        statusPet = "NORMAL";
        if (temperatura > 30.0) {
          interpretacao = "O pet dormiu bem, mas o ambiente registrou temperatura elevada.";
        } else if (temperatura < 15.0) {
          interpretacao = "O pet descansou, mas o ambiente registrou baixa temperatura.";
        } else {
          interpretacao = "Excelente! Sono de qualidade e ambiente em condicoes ideais.";
        }
        digitalWrite(LED_ALERTA, LOW);
      } 
      else if (scoreDescanso >= 50) {
        statusPet = "ATENCAO";
        if (interrupcoes >= 7) {
          interpretacao = "Sono muito fragmentado. Monitore possiveis perturbacoes no local.";
        } else if (totalSono < 12) {
          interpretacao = "O tempo total de repouso ficou abaixo do recomendado.";
        } else if (totalSono > 18) {
          interpretacao = "Tempo de repouso excessivo. Verifique se o pet apresenta desanimo.";
        } else {
          interpretacao = "As condicoes climaticas locais prejudicaram a qualidade do sono.";
        }
        digitalWrite(LED_ALERTA, LOW);
      } 
      else {
        statusPet = "CRITICO";
        if (totalSono < 5 && interrupcoes >= 12) {
          interpretacao = "Alerta! Pouco tempo de descanso e alto indice de agitacao.";
        } else if (totalSono < 5) {
          interpretacao = "Alerta! O animal passou a maior parte do ciclo sem repousar.";
        } else {
          interpretacao = "Preocupante. Sono insuficiente e clima inadequado afetaram o bem-estar.";
        }
        piscarAlerta();
      }

      thing.stream("score_descanso");
      thing.stream("temperatura_pet");
      thing.stream("umidade_pet");
      thing.stream("status_clinico");
      thing.stream("analise_ia");
      thing.stream("total_sono");
      thing.stream("interrupcoes");
      thing.stream("media_sono");

      Serial.print("\n > Tempo Total de Sono   : "); Serial.print(totalSono); Serial.println(" horas simuladas");
      Serial.print(" > Sessoes de Sono       : "); Serial.println(sessoesSono);
      Serial.print(" > Interrupcoes (Acordou): "); Serial.println(interrupcoes);
      Serial.print(" > Media por Sessao      : "); Serial.print(mediaSono, 1); Serial.println(" horas simuladas");
      Serial.println("--------------------------------------------------");
      Serial.print(" > Temperatura Ambiente  : "); Serial.print(temperatura, 1); Serial.println(" C");
      Serial.print(" > Umidade do Air        : "); Serial.print(umidade, 1); Serial.println(" %");
      Serial.println("--------------------------------------------------");
      Serial.print(" [SCORE DE DESCANSO]     : "); Serial.print(scoreDescanso); Serial.println(" / 100");
      Serial.print(" [STATUS DO PET]         : "); Serial.println(statusPet);
      Serial.print(" [ANALISE CLINICA]       : "); Serial.println(interpretacao);
      Serial.println("\nFIM DO DIA SIMULADO. Relatorio enviado para a nuvem.");
      Serial.print("Aguardando clique no Botao Iniciar para comecar o proximo dia...");

      totalSono = 0;
      sessoesSono = (dormindo) ? 1 : 0; 
      interrupcoes = 0;
      mediaSono = 0;
    }
  }

  ultimoBotaoDeitar = leituraDeitar;
  ultimoBotaoLevantar = leituraLevantar;

  delay(10);
}