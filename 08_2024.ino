#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <SFE_BMP180.h>
#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;
SFE_BMP180 pressure;

#define ALTITUDE 1655.0 // Altitude em metros

int lcdColumns = 20;
int lcdRows = 4;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char* SSID = "Okura&Aguiar";
const char* PWD = "*welcome03";

AsyncWebServer server(80);

const uint8_t MQ = 35;
int mq135Value = 0;

// Variáveis globais
double T = 0.0;
double P = 0.0;
sensors_event_t humidity;  // Variável global para umidade
sensors_event_t temp;      // Variável global para temperatura


void setup() {
  Serial.begin(115200);

  aht.begin();
  pressure.begin();
  lcd.init();
  lcd.backlight();

  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);

  WiFi.begin(SSID, PWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Falha na alocação do SSD1306"));
    while (true);
  }

  setupServer();
}

void loop() {
  char status;
  double p0, a;
  
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  
  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");


  // Leitura da temperatura do BMP180
  status = pressure.startTemperature();
  if (status != 0) {
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0) {
      Serial.print("Temperatura BMP180: ");
      Serial.print(T, 2);
      Serial.println(" deg C");
    } else {
      Serial.println("Erro ao obter a temperatura.");
    }
  } else {
    Serial.println("Erro ao iniciar a medição de temperatura.");
  }

  // Leitura da pressão do BMP180
  status = pressure.startPressure(3);
  if (status != 0) {
    delay(status);
    status = pressure.getPressure(P, T);
    if (status != 0) {
      Serial.print("Pressão BMP180: ");
      Serial.print(P, 2);
      Serial.println(" hPa");

      p0 = pressure.sealevel(P, ALTITUDE);
      Serial.print("Pressão ao nível do mar: ");
      Serial.print(p0, 2);
      Serial.println(" hPa");

      a = pressure.altitude(P, p0);
      Serial.print("Altitude: ");
      Serial.print(a, 2);
      Serial.println(" metros");
    } else {
      Serial.println("Erro ao obter a pressão.");
    }
  } else {
    Serial.println("Erro ao iniciar a medição de pressão.");
  }

  // Leitura do sensor MQ135
  mq135Value = analogRead(MQ);

  // Atualiza LCD
  lcd.setCursor(0, 0);
  lcd.print("Tb:");
  lcd.print(T, 2);
  lcd.print(" Ta:");
  lcd.print(temp.temperature, 2);

  lcd.setCursor(0, 1);
  lcd.print("P: ");
  lcd.print(P, 2);
  lcd.print(" Ua:");
  lcd.print(humidity.relative_humidity, 2);

  lcd.setCursor(0, 2);
  lcd.print("MQ135: ");
  lcd.print(mq135Value);
  
  lcd.setCursor(0, 3);
  lcd.print(WiFi.localIP());

  // Atualiza OLED
  displayData();

  delay(1000);
}

void setupServer() {
  server.on("/led1/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(12, HIGH);  // Ligar o LED
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonDocument(200);
    jsonDocument["type"] = "LED1 LIGADO";
    serializeJson(jsonDocument, *response);
    request->send(response);
  });

  server.on("/led1/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(12, LOW);  // Desligar o LED
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonDocument(200);
    jsonDocument["type"] = "LED1 DESLIGADO";
    serializeJson(jsonDocument, *response);
    request->send(response);
  });

  server.on("/led2/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(13, HIGH);  // Ligar o LED
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonDocument(200);
    jsonDocument["type"] = "LED2 LIGADO";
    serializeJson(jsonDocument, *response);
    request->send(response);
  });

  server.on("/led2/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(13, LOW);  // Desligar o LED
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonDocument(200);
    jsonDocument["type"] = "LED2 DESLIGADO";
    serializeJson(jsonDocument, *response);
    request->send(response);
  });

  server.on("/led3/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(14, HIGH);  // Ligar o LED
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonDocument(200);
    jsonDocument["type"] = "LED3 LIGADO";
    serializeJson(jsonDocument, *response);
    request->send(response);
  });

  server.on("/led3/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(14, LOW);  // Desligar o LED
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonDocument(200);
    jsonDocument["type"] = "LED3 DESLIGADO";
    serializeJson(jsonDocument, *response);
    request->send(response);
  });

  server.on("/bmp_temp", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonDocument(200);
    double T;
    char status = pressure.startTemperature();
    if (status != 0) {
      delay(status);
      status = pressure.getTemperature(T);
      if (status != 0) {
        jsonDocument["type"] = "temperature bmp180";
        jsonDocument["value"] = T;
        jsonDocument["unit"] = "C";
      } else {
        jsonDocument["type"] = "error";
        jsonDocument["message"] = "Erro ao obter a temperatura.";
      }
    } else {
      jsonDocument["type"] = "error";
      jsonDocument["message"] = "Erro ao iniciar a medição de temperatura.";
    }
    serializeJson(jsonDocument, *response);
    request->send(response);
  });

  server.on("/bmp_pressao", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonDocument(200);
    double T, P;
    char status = pressure.startTemperature();
    if (status != 0) {
      delay(status);
      status = pressure.getTemperature(T);
      if (status != 0) {
        status = pressure.startPressure(3);
        if (status != 0) {
          delay(status);
          status = pressure.getPressure(P, T);
          if (status != 0) {
            jsonDocument["type"] = "pressure bmp180";
            jsonDocument["value"] = P;
            jsonDocument["unit"] = "hPa";
          } else {
            jsonDocument["type"] = "error";
            jsonDocument["message"] = "Erro ao obter a pressão.";
          }
        } else {
          jsonDocument["type"] = "error";
          jsonDocument["message"] = "Erro ao iniciar a medição de pressão.";
        }
      } else {
        jsonDocument["type"] = "error";
        jsonDocument["message"] = "Erro ao obter a temperatura.";
      }
    } else {
      jsonDocument["type"] = "error";
      jsonDocument["message"] = "Erro ao iniciar a medição de temperatura.";
    }
    serializeJson(jsonDocument, *response);
    request->send(response);
  });

  server.on("/dados", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonDocument(200);

    double T, P;
    char status = pressure.startTemperature();
    if (status != 0) {
      delay(status);
      status = pressure.getTemperature(T);
      if (status != 0) {
        status = pressure.startPressure(3);
        if (status != 0) {
          delay(status);
          status = pressure.getPressure(P, T);
          if (status != 0) {
            int mq135Value = analogRead(MQ);
            jsonDocument["temperature"] = T;
            jsonDocument["pressure"] = P;
            jsonDocument["mq135"] = mq135Value;
            aht.getEvent(&humidity, &temp);
            jsonDocument["aht_temperature"] = temp.temperature;
            jsonDocument["aht_humidity"] = humidity.relative_humidity;
          } else {
            jsonDocument["type"] = "error";
            jsonDocument["message"] = "Erro ao obter a pressão.";
          }
        } else {
          jsonDocument["type"] = "error";
          jsonDocument["message"] = "Erro ao iniciar a medição de pressão.";
        }
      } else {
        jsonDocument["type"] = "error";
        jsonDocument["message"] = "Erro ao obter a temperatura.";
      }
    } else {
      jsonDocument["type"] = "error";
      jsonDocument["message"] = "Erro ao iniciar a medição de temperatura.";
    }

    serializeJson(jsonDocument, *response);
    request->send(response);
  });

  server.on("/aht_temp", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonDocument(200);
    aht.getEvent(&humidity, &temp);
    jsonDocument["type"] = "temperature ahtx0";
    jsonDocument["value"] = temp.temperature;
    jsonDocument["unit"] = "C";
    serializeJson(jsonDocument, *response);
    request->send(response);
  });

  server.on("/aht_humid", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument jsonDocument(200);
    aht.getEvent(&humidity, &temp);
    jsonDocument["type"] = "humidity ahtx0";
    jsonDocument["value"] = humidity.relative_humidity;
    jsonDocument["unit"] = "%";
    serializeJson(jsonDocument, *response);
    request->send(response);
  });

  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    displayData();
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", display.getBuffer(), display.width() * display.height());
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    displayDataOff();
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", display.getBuffer(), display.width() * display.height());
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.begin();
}

void displayData() {

  // Atualiza OLED
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("IP: ");
  display.print(WiFi.localIP());

  display.setTextSize(1);
  display.setCursor(0, 30);
  //display.print("AHT H");
//  display.print(temp.temperature);

  display.setTextSize(2);
  display.setCursor(0, 40);
  //display.print(humidity.relative_humidity);
  
  display.display();
}

void displayDataOff() {
  display.clearDisplay();
  display.display();
}
