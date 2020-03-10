#include <ESP8266WiFi.h>
#include <PubSubClient.h> 
#include <ArduinoJson.h>

const char* ssid = "Your SSID";
const char* password = "Your Password";

#define ORG "xxxx"                    // IBM blumix org ID
#define DEVICE_TYPE "WeatherStation"  // Device type
#define DEVICE_ID "W01"               // Device ID
#define TOKEN "xxxxxyyyyyzzzzz"    // Your auth token


char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

const char eventTopic[] = "iot-2/evt/status/fmt/json";
const char cmdTopic[] = "iot-2/cmd/led/fmt/json";

WiFiClient wifiClient;

void callback(char* topic, byte* payload, unsigned int payloadLength) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < payloadLength; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if (payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);
    Serial.println("false");
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}

PubSubClient client(server, 1883, callback, wifiClient);

long publishInterval = 30000;
long lastPublishMillis;

void setup() {
  
  Serial.begin(115200); Serial.println();  
  client.connect(clientId, authMethod, token);
  pinMode(LED_BUILTIN, OUTPUT);
  wifiConnect();
  mqttConnect();
}

void loop() {
  
  if (millis() - lastPublishMillis > publishInterval) {
    publishData();
    lastPublishMillis = millis();
  }

  if (!client.loop()) {
    mqttConnect();
  }
}

void wifiConnect() {
  
  Serial.print("Connecting to "); Serial.print(ssid);
  
  WiFi.begin(ssid, password);
  
  int i = 0;
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Connected to WiFi");
    }
    Serial.println(i);
    i++;
    if (i > 60)
    {
      ESP.restart();
    }
  }
  Serial.print("WiFi connected, IP address: "); Serial.println(WiFi.localIP());
}

void mqttConnect() {
  if (!!!client.connected()) {
    
    Serial.print("Reconnecting MQTT client to "); Serial.println(server);
    
    while (!!!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }
    if (client.subscribe(cmdTopic)) {
      Serial.println("subscribe to responses OK");
    } else {
      Serial.println("subscribe to responses FAILED");
    }
    Serial.println();
  }
}

void publishData() {
  
  String payload;
  float temp = 67;
  float hum = 76;

  StaticJsonDocument<200> doc; //create json document named doc
  
  doc["temp"] = temp;          //add members to json
  doc["hum"] = hum;           

  serializeJson(doc, payload); //Serialize json
  
  Serial.print("Sending payload: "); Serial.println(payload);

  if (client.publish(eventTopic, (char*) payload.c_str())) {
    Serial.println("Publish OK");
  } else {
    Serial.println("Publish FAILED");
  }
}
