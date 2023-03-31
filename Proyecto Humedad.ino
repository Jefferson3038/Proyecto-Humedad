#include <LiquidCrystal_I2C.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include <RTCZero.h>

char ssid[] = "Grupo2";       
char pass[] = "12345678";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

int port = 1883;
const char broker[] = "broker.mqttdashboard.com";
const char topic[]  = "HumedadSensor100";
const char topic2[]  = "BombaPoderosa100";
const char topic3[] = "encenderBomba100";

RTCZero rtc;

/* Change these values to set the current initial time */
const int year = 23;
const byte day = 31;
const byte month = 3;

LiquidCrystal_I2C LCD(0x27, 16, 2); // Dirección I2C 0x27, 16 columnas y 2 filas
const int sensorHumedad = A1;
int RELAY = 0;
int bombaEstado = 0;
const long interval = 5000;
unsigned long previousMillis = 0;

void setup(){
  Serial.begin(9600);
  LCD.init(); //inicializar la pantalla LCD  LCD. luz de fondo(); //abrir la luz de fondo 
  LCD.backlight();
  pinMode(RELAY,OUTPUT);
  digitalWrite(RELAY,HIGH);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  mqttClient.onMessage(onMqttMessage);
  mqttClient.subscribe(topic3);

  mqttClient.beginMessage(topic2);
  mqttClient.print(0);
  mqttClient.endMessage();

  rtc.begin(); // initialize RTC

  // Set the time
  rtc.setDate(day,month,year);

}

void loop(){
  int humedad = analogRead(sensorHumedad);
  int porcentageH = -0.12135*humedad+124.271844;
  char varibale[10];
  sprintf(varibale, "%d%%", porcentageH);
  mqttClient.poll();
  delay(2000);
  mqttClient.beginMessage(topic);
  mqttClient.print(varibale);
  mqttClient.endMessage();
  mqttClient.beginMessage(topic2);
  mqttClient.print(bombaEstado);
  mqttClient.endMessage();

  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("Huemdad:");
  LCD.setCursor(8, 0);
  LCD.print(porcentageH);
  LCD.setCursor(10, 0);
  LCD.print("%");
  LCD.setCursor(0,1);

  Serial.println(humedad);

  if(humedad<750){
    bombaEstado = 0;
    digitalWrite(RELAY, HIGH);
    LCD.print(rtc.getDay());
    LCD.print("/"); //retrieve hours
    LCD.print(rtc.getMonth()); //retrieve minutes
    LCD.print("/"); //retrieve hours
    LCD.print(rtc.getYear()); 
  }
  else{
    bombaEstado = 1;
    digitalWrite(RELAY, LOW);
    LCD.print("Regando...");
    delay(2500);
    digitalWrite(RELAY, HIGH);
  }
  delay(1000);
}



void onMqttMessage(int messageSize) {
  Serial.println("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");
  if(mqttClient.read()>0){
    LCD.clear();
    digitalWrite(RELAY, LOW);
    LCD.print("Activando");
    LCD.setCursor(0, 1);
    LCD.print("Bomba");
    delay(2500);
    digitalWrite(RELAY, HIGH);
  }

}