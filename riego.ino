inc#lude <DHT.h>
#include <TimerOne.h>
#include <SoftwareSerial.h>
#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>

#define ESP8266_BAUD 9600
#define bomba 12
#define sensorLuz A0
#define sensorAgua A1
#define DHTPIN 11 //sensor temperatura
#define sensorHumedad A2
#define ledAlto 8
#define ledMedio 7 
#define ledBajo 6

#define BLYNK_PRINT Serial

DHT dht (DHTPIN, DHT11);// Inicializamos el sensor 
SoftwareSerial EspSerial(2, 3); // RX, TX
ESP8266 wifi(&EspSerial);
BlynkTimer timer;

WidgetLED LedAlto(V4);
WidgetLED LedMedio(V5);
WidgetLED LedBajo(V6);
int cantidadAgua = 1;
int contador = 0;
int contadorTiempo = 0;
int tiempoFinal = 0;
int tiempoInicio = 0;
int tempTemp = 0;
int humedadSuelo = 0;
float temperatura = 0;
float sensacionTermica = 0;
bool auxYaLuz = false;
bool hayAgua = true;
char auth[] = "161d5c1e88374aaa9620fe8696e970e2";
char ssid[] = "Calidonio 2.4";
char pass[] = "SEBASTIANdavid1234";

void setup()
{
  EspSerial.begin(ESP8266_BAUD);
  delay(10);
  Blynk.begin(auth, wifi, ssid, pass);
  
  pinMode(bomba, OUTPUT);
  pinMode(ledAlto, OUTPUT);
  pinMode(ledMedio, OUTPUT);
  pinMode(ledBajo, OUTPUT);
  
  pinMode(sensorLuz,INPUT);
  pinMode(sensorAgua,INPUT);
  pinMode(sensorHumedad,INPUT);

  dht.begin(); //pinMode(sensorTemp,INPUT);
  
  timer.setInterval(1000L, imprimirInformacion);

  Serial.begin(9600);
}

void logica() {
  // put your main code here, to run repeatedly:
  ////////////////// Sensor de Agua ///////////////////
  cantidadAgua = analogRead(sensorAgua);
  if(analogRead(sensorAgua)  > 200){
    hayAgua = true;
  }else{
    hayAgua = false;
  }
  //////////////////// Sensor de Humedad ///////////////////////
  //////////////////// Activar Bomba //////////////////////////
  tempTemp = analogRead(sensorHumedad);
  tempTemp = map(tempTemp,0,1023,0,100);
  humedadSuelo = tempTemp;
  if(tempTemp > 50 && tempTemp <= 100){
    regarPlanta();
  }else if(tempTemp>=0 && tempTemp <= 30){
    digitalWrite(bomba, LOW);
    delay(100);
  }
  ////////////////// Activar Luces de Estado ///////////////////
  if(tempTemp>=0 && tempTemp <= 30){
    LedAlto.on();
    LedMedio.off();
    LedBajo.off();
    digitalWrite(ledAlto, HIGH);
    digitalWrite(ledMedio, LOW);
    digitalWrite(ledBajo, LOW);
  }else if(tempTemp>=40 && tempTemp <= 60){
    LedAlto.off();
    LedMedio.on();
    LedBajo.off();
    digitalWrite(ledAlto, LOW);
    digitalWrite(ledMedio, HIGH);
    digitalWrite(ledBajo, LOW);
  }else if(tempTemp>=70 && tempTemp <= 100){
    LedAlto.off();
    LedMedio.off();
    LedBajo.on();
    digitalWrite(ledAlto, LOW);
    digitalWrite(ledMedio, LOW);
    digitalWrite(ledBajo, HIGH);
  }
  ////////////////// Sensor de Luz ////////////////////////////
  /////// Cuenta las horas de luz que recibe la planta ///////
  if (digitalRead(sensorLuz) == HIGH){
    if(auxYaLuz){
      //tiempoFinal = millis() / 1000;
      //contadorTiempo += ((tiempoFinal - tiempoInicio)/3600);
    }else{
      tiempoInicio = millis() / 1000;
      auxYaLuz = true;
    }
  }else if(digitalRead(sensorLuz) == LOW){
    if(auxYaLuz){
      tiempoFinal = millis() / 1000;
      contadorTiempo = ((tiempoFinal - tiempoInicio));
      auxYaLuz = false;
      tiempoInicio = 0;
      tiempoFinal = 0;
    }else{
      //tiempoInicio = millis() / 1000;
      //auxYaLuz = true;
    }
  }
  ///////////////// Sensor de Temperatura /////////////////////
  //////////////// Mide la temperatura actual ////////////////
  calcularTemperatura();

  ///////////////// Mandar Información /////////////////////
  //imprimirInformacion();
}

void loop() {
  logica();
  Blynk.run();
  timer.run(); // Initiates BlynkTimer
}
void regarPlanta(){
    digitalWrite(bomba, HIGH);
    delay(500);   
}

void calcularTemperatura(){
  // Leemos la humedad relativa
  float h = dht.readHumidity();
  // Leemos la temperatura en grados centígrados
  temperatura = dht.readTemperature();
 // Calcular el índice de calor en grados centígrados
  sensacionTermica = dht.computeHeatIndex(temperatura, h, false);
  if (isnan(h) || isnan(temperatura)) {temperatura=0; sensacionTermica=0;}
}

void imprimirInformacion(){
  Blynk.virtualWrite(V0, contadorTiempo);
  Blynk.virtualWrite(V1, temperatura);
  Blynk.virtualWrite(V2, sensacionTermica);
  Blynk.virtualWrite(V3, humedadSuelo);
}
