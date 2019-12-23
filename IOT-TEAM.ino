#include <DHT.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Wire.h>


LiquidCrystal_I2C lcd(0x27, 16, 2);

//kết nối wifi
const char* ssid     = "TRUNG TIN";
const char* password = "12345678";

//kết nối WebApp
const char* serverName = "http://iot-team-f2g.herokuapp.com/database.php";
String apiKeyValue = "tPmAT5Ab3j7F9";

//kết nối websocket
#define mqtt_server "tailor.cloudmqtt.com"
#define mqtt_topic_sub "home/garden/fountain"
#define mqtt_user "jwlfxwbw"
#define mqtt_pwd "xaoizh2liQ2y"

#define DHTTYPE DHT11
// DHT Sensor
#define dht_dpin 0
DHT dht(dht_dpin, DHTTYPE); 

int Temperature;
int Humidity;

WiFiClient espClient;
PubSubClient client(espClient);

float celsius;
unsigned long time1 = 0;

void setup() {
  Serial.begin(115200);
  //open LCD
  lcd.init();
  lcd.backlight();
  //wifi
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  //web socket
  client.setServer(mqtt_server, 15313);
  client.setCallback(callback);
  dht.begin();   
}

void callback(char* topic, byte* payload, unsigned int length) {
  lcd.setCursor(0, 0);
  lcd.print("                ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    lcd.setCursor(i, 0);
    lcd.print((char)payload[i]);
  }
}

//
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_user, mqtt_pwd)) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe(mqtt_topic_sub);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  if ( (unsigned long) (millis() - time1) > 10000 )
  {
    Temperature = dht.readTemperature(); // Gets the values of the temperature
    Humidity = dht.readHumidity();
    Serial.print(Temperature); 
    Serial.print(Humidity);
    lcd.setCursor(0, 1);
    lcd.print("T:" );
    lcd.print(Temperature);
    lcd.write(223);
    lcd.print("C");
    lcd.print(" - H:");
    lcd.print(Humidity);
     lcd.print("%");
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverName);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      String httpRequestData = "api_key=" + apiKeyValue + "&Humidity=" + String(Humidity) + "&Temperature=" + String(Temperature) + "";
      Serial.print("httpRequestData: ");
      Serial.println(httpRequestData);
      int httpResponseCode = http.POST(httpRequestData);
      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    time1 = millis();
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
