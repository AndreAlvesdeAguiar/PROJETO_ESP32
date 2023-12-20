#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <DHT.h>

const char *SSID = "Okura&Aguiar";
const char *PWD = "*welcome03";

#define DHTTYPE DHT22   // DHT 11

const int red_pin = 27;   
const int green_pin = 13; 
const int blue_pin = 12;
const int yellow_pin = 19; 
 
const int analogInPin1 = 36;
const int analogInPin2 = 39;
const int analogInPin3 = 34;


const int DHTPin = 25;

// Setting PWM frequency, channels and bit resolution
const int frequency = 5000;
const int redChannel = 0;
const int greenChannel = 1;
const int blueChannel = 2;

const int yellowChannel = 3;

const int resolution = 8;

WebServer server(80);
 
DHT dht(DHTPin, DHTTYPE);

StaticJsonDocument<500> jsonDocument;
char buffer[500];

float temperature;
float humidity;

float moisture1;
float moisture2;

float mq135;
//float pressure;
 
void setup_routing() {     
  server.on("/temperature", getTemperature);     
  //server.on("/pressure", getPressure);     
  server.on("/humidity", getHumidity);   
  server.on("/moisture1", getMoisture1);     
  server.on("/moisture2", getMoisture2);     
  server.on("/mq135", getMq135);     

  server.on("/data", getData);     
  server.on("/led", HTTP_POST, handlePost);    
          
  server.begin();    
}
 
void create_json(char *tag, float value, char *unit) {  
  jsonDocument.clear();  
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);
}
 
void add_json_object(char *tag, float value, char *unit) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
  obj["unit"] = unit; 
}

void read_sensor_data(void * parameter) {
   for (;;) {
     temperature = dht.readTemperature();
     humidity = dht.readHumidity();
     moisture1 = analogRead(analogInPin1);
     moisture2 = analogRead(analogInPin2);
     mq135 = analogRead(analogInPin3);

  //   pressure = bme.readPressure() / 100;
   Serial.println("Read sensor data");
   Serial.println(temperature);
   Serial.println(humidity);
   Serial.println(moisture1);
   Serial.println(moisture2);
   Serial.println(mq135);

     vTaskDelay(60000 / portTICK_PERIOD_MS);
   }
}
 
void getTemperature() {
  Serial.println("Get temperature");
  create_json("temperature", temperature, "°C");
  server.send(200, "application/json", buffer);
}

void getHumidity() {
  Serial.println("Get humidity");
  create_json("humidity", humidity, "%");
  server.send(200, "application/json", buffer);
}
 
void getMoisture1() {
  Serial.println("Get humidity");
  create_json("moisture1", moisture1, "%");
  server.send(200, "application/json", buffer);
}

void getMoisture2() {
  Serial.println("Get humidity");
  create_json("moisture2", moisture2, "%");
  server.send(200, "application/json", buffer);
}

void getMq135() {
  Serial.println("Get humidity");
  create_json("mq135", mq135, "%");
  server.send(200, "application/json", buffer);
}
 
void getData() {
  Serial.println("Get a Sensors Data");
  jsonDocument.clear();
  add_json_object("temperature", temperature, "°C");
  add_json_object("humidity", humidity, "%");
  add_json_object("moisture1", moisture1, "%");
  add_json_object("moisture2", moisture2, "%");
  add_json_object("mq135", mq135, "%");

 // add_json_object("pressure", pressure, "hPa");
  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}

void handlePost() {
  if (server.hasArg("plain") == false) {
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  int red_value = jsonDocument["red"];
  int green_value = jsonDocument["green"];
  int blue_value = jsonDocument["blue"];
  int yellow_value = jsonDocument["yellow"];


  ledcWrite(redChannel, red_value);
  ledcWrite(greenChannel,green_value);
  ledcWrite(blueChannel, blue_value);
  ledcWrite(yellowChannel, yellow_value);


  server.send(200, "application/json", "{}");
}

void setup_task() {    
  xTaskCreate(     
  read_sensor_data,      
  "Read sensor data",      
  1000,      
  NULL,      
  1,     
  NULL     
  );     
}

void setup() {     
  Serial.begin(115200); 
  dht.begin();

  ledcSetup(redChannel, frequency, resolution);
  ledcSetup(greenChannel, frequency, resolution);
  ledcSetup(blueChannel, frequency, resolution);
  ledcSetup(yellowChannel, frequency, resolution);

 
  ledcAttachPin(red_pin, redChannel);
  ledcAttachPin(green_pin, greenChannel);
  ledcAttachPin(blue_pin, blueChannel);
  ledcAttachPin(yellow_pin, yellowChannel);
         
 /* if (!bme.begin(0x76)) {    
    Serial.println("BME280 not found! Check Circuit");    
  } */    

  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(SSID, PWD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
 
  Serial.print("Connected! IP Address: ");
  Serial.println(WiFi.localIP());
  setup_task();    
  setup_routing();     
   
}    
       
void loop() {    
  server.handleClient();   
  Serial.println("-----------------");
  Serial.println("Log Sensors");
  Serial.print("Temperature - ");
  Serial.println(temperature);
  Serial.print("Humidity - ");
  Serial.println(humidity);
  Serial.print("Moisture1 - ");
  Serial.println(moisture1);
  Serial.print("Moisture2 - ");
  Serial.println(moisture2);
  Serial.print("Mq135 [Co2] - ");
  Serial.println(mq135); 
  Serial.println("-----------------");
 
  delay(1000);
}
