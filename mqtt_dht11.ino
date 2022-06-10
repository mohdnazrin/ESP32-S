
//############################################################################
//####  This program consist of:                                          #### 
//####  ModeMCU-32 (ESP32 Dev Module) or (NoeMCU-32S)                     #### 
//####  DHT11 Temperature and Humidity sensor                             #### 
//############################################################################
//#### Pin assignment                                                     ####
//#### 23         signal for DHT11 signal                                 ####
//############################################################################

//######################Wifi for ESP #######################################//
#include <WiFi.h>
//######################MQTT for ESP #######################################//
#include <PubSubClient.h>

//######### DHT Library   ####################################################//
#include "DHT.h"

//######### DHT22 Pins assignment (3 pins) ###################################//
#define DHTPIN 23//middle pin of sensor connected to ESP32
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


// Change the credentials below, so your ESP8266 connects to your WiFi router
const char* ssid = "";
const char* password = "";

// Change the variable to your IP address, so it connects to your MQTT broker.
//get IP address on your raspberryPi or localhost machine
const char* mqtt_server = "xx.xxx.xxx.xx";


//Authetication to MQTT if available
//const char* mqttUser = "mqtt-user";
//const char* mqttPassword = "en020705";

// Initializes the espClient.
WiFiClient espClient_esp32;
PubSubClient client(espClient_esp32);


// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;

// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
  
}

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messagefromNodeRed;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messagefromNodeRed += (char)message[i];
  }
  Serial.println();
  
    
  // Feel free to add more if statements to control more GPIOs with MQTT

    // If a message is received on the topic resetESP32, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(topic=="resetESP32"){
      Serial.print("Reset ESP32 ");
      if(messagefromNodeRed == "on"){
        ESP.restart();
        Serial.print("reset");
      }
      else if(messagefromNodeRed == "off"){
        Serial.print("non-reset");
      }
  }

  
  Serial.println();  
}
// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32")) { //when no user & password required
    //if (client.connect("ESP32", mqttUser, mqttPassword)) {      //Authetication to MQTT if required
      Serial.println("connected");  
      
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
     


    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the LEDs
void setup() {

  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  dht.begin();
  

}

// For this project, you don't need to change anything in the loop function. Basically it ensures that you ESP is connected to your broker
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP32");

  now = millis();
  // Publishes new temperature and humidity every 1 seconds
  if (now - lastMeasure > 4000) {
    lastMeasure = now;

    
  //*************read DHT11 Keyes sensor*****************************************
  float temperature = dht.readTemperature();  // read your temperature sensor to execute temperature compensation
       // Check if any reads failed and exit early (to try again).
    if ( isnan(temperature) ) {
      Serial.println("Failed to read from DHT11 sensor!");
      return;
    }
    float humidity = dht.readHumidity();
    if ( isnan(humidity) ) {
      Serial.println("Failed to read from DHT11 sensor!");
      return;
    }


  float temp_DHT11=temperature; 
  static char temp_reading[20];
  dtostrf(temp_DHT11, 6, 2, temp_reading);    

  float hum_DHT11=humidity; 
  static char hum_reading[20];
  dtostrf(hum_DHT11, 6, 2, hum_reading);   


             
    //*********** Publishes MQTT Topic *******************************************
    client.publish("temp_DHT11_esp32",temp_reading);
    client.publish("hum_DHT11_esp32",hum_reading);


    //************ to print all sensors via Serial Monitor ***********************
    Serial.print("\n");   
    Serial.print("temperature:");
    Serial.print(temperature, 1);
    Serial.println("^C");
    
    Serial.print("humidity:");
    Serial.print(humidity, 1);
    Serial.println("^%");   
    

  }
  
  }
  

  
