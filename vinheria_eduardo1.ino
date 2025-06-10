#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define col 16
#define lin  2
#define ende  0x27

const int ledVerdePin = 13;
const int ledAmareloPin = 12;
const int ledVermelhoPin = 11;
const int buzzerPin = 10;
const int ldrPin = A1;
const int tempPin = A2;
const int umidPin = A0;

LiquidCrystal_I2C lcd(ende, col, lin);

const float TEMP_MIN = 10.0;
const float TEMP_MAX = 15.0;
const int UMID_MIN = 50;
const int UMID_MAX = 70;

const int LUZ_ESCURO = 880;
const int LUZ_MEIA_LUZ = 960;
const int NUM_LEITURAS = 2;

float leituras_temp[NUM_LEITURAS];
int leituras_umid[NUM_LEITURAS];
int leituras_luz[NUM_LEITURAS];
int indice_leituras = 0;
bool buffer_cheio = false;

unsigned long tempo_anterior = 0;
const unsigned long INTERVALO = 5000;

float temperatura = 0.0;
int umidade = 0;
int luminosidade = 0;

float lerTemperatura() {
  int valorSensor = analogRead(tempPin);
  Serial.print("Valor sensor temp: ");
  Serial.println(valorSensor);

  return (valorSensor / 1023.0) * 45.0;  // Retorna valor com precisão entre 0.0 e 40.0 °C
}

int lerUmidade() {
  int valorSensor = analogRead(umidPin);
  return map(valorSensor, 0, 1023, 0, 100);
}

void calcularMedias() {
  float soma_temp = 0;
  int soma_umid = 0;
  int soma_luz = 0;
  int total_amostras = buffer_cheio ? NUM_LEITURAS : indice_leituras;

  for (int i = 0; i < total_amostras; i++) {
    soma_temp += leituras_temp[i];
    soma_umid += leituras_umid[i];
    soma_luz += leituras_luz[i];
  }

  temperatura = soma_temp / total_amostras;
  umidade = soma_umid / total_amostras;
  luminosidade = soma_luz / total_amostras;
}

void verificarLuminosidade() {
  digitalWrite(ledVerdePin, LOW);
  digitalWrite(ledAmareloPin, LOW);
  digitalWrite(ledVermelhoPin, LOW);
  
  if (luminosidade < LUZ_ESCURO) {
    digitalWrite(ledVerdePin, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ambiente escuro");
    digitalWrite(buzzerPin, LOW);
  }
  else if (luminosidade < LUZ_MEIA_LUZ) {
    digitalWrite(ledAmareloPin, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ambiente a meia");
    lcd.setCursor(0, 1);
    lcd.print("luz");
    digitalWrite(buzzerPin, LOW);
  }
  else {
    digitalWrite(ledVermelhoPin, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ambiente muito");
    lcd.setCursor(0, 1);
    lcd.print("claro");
    digitalWrite(buzzerPin, HIGH);
  }

  // DEBUG: mostra valor real
  Serial.print("Luminosidade bruta: ");
  Serial.println(luminosidade);
}

void verificarTemperatura() {
  if (temperatura >= TEMP_MIN && temperatura <= TEMP_MAX) {
    digitalWrite(ledAmareloPin, LOW);
    digitalWrite(buzzerPin, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temperatura OK");
    lcd.setCursor(0, 1);
    lcd.print("Temp: ");
    lcd.print(temperatura);
    lcd.print(" C");
  } 
  else {
    digitalWrite(ledAmareloPin, HIGH);
    digitalWrite(buzzerPin, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(temperatura < TEMP_MIN ? "Temp. Baixa" : "Temp. Alta");
    lcd.setCursor(0, 1);
    lcd.print("Temp: ");
    lcd.print(temperatura);
    lcd.print(" C");
  }
}

void verificarUmidade() {
  if (umidade >= UMID_MIN && umidade <= UMID_MAX) {
    digitalWrite(ledVermelhoPin, LOW);
    digitalWrite(buzzerPin, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Umidade OK");
    lcd.setCursor(0, 1);
    lcd.print("Umid: ");
    lcd.print(umidade);
    lcd.print("%");
  } 
  else {
    digitalWrite(ledVermelhoPin, HIGH);
    digitalWrite(buzzerPin, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(umidade < UMID_MIN ? "Umidade. Baixa" : "Umidade. Alta");
    lcd.setCursor(0, 1);
    lcd.print("Umid: ");
    lcd.print(umidade);
    lcd.print("%");
  }
}

void setup() {
  pinMode(ledVerdePin, OUTPUT);
  pinMode(ledAmareloPin, OUTPUT);
  pinMode(ledVermelhoPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Vinheria Do Edu");
  lcd.setCursor(0, 1);
  lcd.print("Inicializando...");

  Serial.begin(9600);

  for (int i = 0; i < NUM_LEITURAS; i++) {
    leituras_temp[i] = 0;
    leituras_umid[i] = 0;
    leituras_luz[i] = 0;
  }

  delay(2000);
}

void loop() {
  float nova_temp = lerTemperatura();
  int nova_umid = lerUmidade();
  int nova_luz = analogRead(ldrPin);

  leituras_temp[indice_leituras] = nova_temp;
  leituras_umid[indice_leituras] = nova_umid;
  leituras_luz[indice_leituras] = nova_luz;

  indice_leituras++;
  if (indice_leituras >= NUM_LEITURAS) {
    indice_leituras = 0;
    buffer_cheio = true;
  }

  calcularMedias();

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.print(" °C, Umidade: ");
  Serial.print(umidade);
  Serial.print("%, Luminosidade: ");
  Serial.println(luminosidade);

  unsigned long tempo_atual = millis();

  static int estado_display = 0;
  if (tempo_atual - tempo_anterior >= INTERVALO || tempo_anterior == 0) {
    tempo_anterior = tempo_atual;
    estado_display = (estado_display + 1) % 3;

    switch (estado_display) {
      case 0:
        verificarLuminosidade();
        break;
      case 1:
        verificarTemperatura();
        break;
      case 2:
        verificarUmidade();
        break;
    }
  }

  delay(1000);
}
