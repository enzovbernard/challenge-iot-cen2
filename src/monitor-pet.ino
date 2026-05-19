#include <WiFi.h>
#include <ThingerESP32.h>
#include "DHTesp.h"

#define USER_ID "enzobernardini"
#define DEVICE_ID "ESP32_Pet_Monitor"
#define DEVICE_CREDENTIAL "MonitorESP32@"

const char* ssid = "Wokwi-GUEST";
const char* password = "";

ThingerESP32 thing(USER_ID, DEVICE_ID, DEVICE_CREDENTIAL);

#define LED_ALERTA 26
#define LED_CONFIRMACAO 25
#define BUTTON_DEITAR 18
#define BUTTON_LEVANTAR 19
#define PIR_PIN 27
#define DHT_PIN 15

DHTesp dhtSensor;

float temperatura = 0;
float umidade = 0;
bool dormindo = false;

bool ultimoBotaoDeitar = HIGH;
bool ultimoBotaoLevantar = HIGH;
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
  if (!dormindo) {
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
  if (dormindo) {
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
  pinMode(BUTTON_DEITAR, INPUT_PULLUP);
  pinMode(BUTTON_LEVANTAR, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT);

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  Serial.println("            INICIALIZANDO MONITOR PET             ");
  Serial.println("       (Escala: 1 Segundo Real = 1 Hora Pet)      ");

  thing.add_wifi(ssid, password);

  thing["status_clinico"] >> [](pson& out){ out = statusPet.c_str(); };
  thing["analise_ia"] >> [](pson& out){ out = interpretacao.c_str(); };
  thing["score_descanso"] >> [](pson& out){ out = scoreDescanso; };
  thing["temperatura_pet"] >> [](pson& out){ out = temperatura; };
  thing["umidade_pet"] >> [](pson& out){ out = umidade; };
  thing["total_sono"] >> [](pson& out){ out = totalSono; };
  thing["interrupcoes"] >> [](pson& out){ out = interrupcoes; };
  thing["media_sono"] >> [](pson& out){ out = mediaSono; };

  inicioDia = millis();
}

void loop() {
  thing.handle();

  bool lecturaDeitar = digitalRead(BUTTON_DEITAR);
  bool leituraLevantar = digitalRead(BUTTON_LEVANTAR);
  bool movimento = digitalRead(PIR_PIN);

  if (millis() - ultimoClima >= 5000) {
    TempAndHumidity data = dhtSensor.getTempAndHumidity();
    temperatura = data.temperature;
    umidade = data.humidity;
    ultimoClima = millis();
  }

  if (lecturaDeitar == LOW && ultimoBotaoDeitar == HIGH) {
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
    if (dormindo) {
      unsigned long tempoParcial = (millis() - inicioSono) / 1000;
      totalSono += tempoParcial;
      inicioSono = millis(); 
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

    if (interrupcoes >= 5) scoreDescanso -= 20;
    else if (interrupcoes >= 3) scoreDescanso -= 10;

    if (temperatura > 30.0) scoreDescanso -= 15; 
    if (temperatura < 15.0) scoreDescanso -= 10; 

    if (scoreDescanso < 0) scoreDescanso = 0;
    if (scoreDescanso > 100) scoreDescanso = 100;

    if (scoreDescanso < 50) {
      statusPet = "CRITICO";
      interpretacao = "Alerta! Severa privacao de sono ou alta agitacao.";
      piscarAlerta();
    } 
    else if (scoreDescanso < 75) {
      statusPet = "ATENCAO";
      interpretacao = "Sono instavel. Monitore o ambiente e o pet.";
      digitalWrite(LED_ALERTA, LOW);
    } 
    else {
      statusPet = "NORMAL";
      interpretacao = "Excelente! O pet esta descansado.";
      digitalWrite(LED_ALERTA, LOW);
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
    Serial.print(" > Umidade do Ar         : "); Serial.print(umidade, 1); Serial.println(" %");
    Serial.println("--------------------------------------------------");
    Serial.print(" [SCORE DE DESCANSO]     : "); Serial.print(scoreDescanso); Serial.println(" / 100");
    Serial.print(" [STATUS DO PET]         : "); Serial.println(statusPet);
    Serial.print(" [ANALISE CLINICA]       : "); Serial.println(interpretacao);
    Serial.println("");
    Serial.print("Começando um novo dia...");

    totalSono = 0;
    sessoesSono = (dormindo) ? 1 : 0; 
    interrupcoes = 0;
    mediaSono = 0;

    inicioDia = millis();
  }

  ultimoBotaoDeitar = lecturaDeitar;
  ultimoBotaoLevantar = leituraLevantar;

  delay(10);
}