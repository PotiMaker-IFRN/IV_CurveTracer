#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

const char* ssid = "wIFRN-IoT";
const char* password = "deviceiotifrn";

const char* mqtt_server = "10.44.1.35";


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

const int ledPin = 2;
const int rele01 = 4;
const int rele02 = 5;

boolean test_run = 0;

const char* voltage_topic = "esp32/voltage";
const char* current_topic = "esp32/current";

// variaveis de amostras
const int amostras = 200;
float voltage[amostras];
float current[amostras];
float leitura_v[amostras];
float leitura_i[amostras];
float raw_volt;

void setup() {
  
  pinMode(rele01, OUTPUT);
  pinMode(rele02, OUTPUT);
  // Inicializa a porta serial
  Serial.begin(9600);

  delay(100);


  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(ledPin, OUTPUT);
}

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
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}



void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp01/led") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(ledPin, HIGH);
      test_run = 1;
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(ledPin, LOW);
    }
  }
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp01/led");
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
    
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

 
  if(test_run) {
    carregaCapacitor();
    test_run = 0;
  }

  

  // Aguarda um pouco antes de começar tudo de novo
  delay(2000);


  
    
}

void carregaCapacitor(){
  digitalWrite(rele01, HIGH);  // DESCARREGA CAP
  digitalWrite(rele02, LOW);  // 
  delay(5000);  
  digitalWrite(rele01, LOW);  // CAPACITOR DESCARREGADO E ISOLADO
  digitalWrite(rele02, LOW);                    
  delay(5000);  
  digitalWrite(rele01, LOW);  // CARREGA CAPACITOR
  digitalWrite(rele02, HIGH);
  //delayMicroseconds(500);                     
  for (int i=0; i<amostras; i++) {
      leitura_v[i]=analogRead(35);
      leitura_i[i]=analogRead(34);
      delayMicroseconds(500);
    }
  delay(4000);

  digitalWrite(rele01, LOW);  // CAPACITOR CARREGADO E ISOLADO
  digitalWrite(rele02, LOW);        

  char voltString[8];
  char currString[8];
  
  for (int i=0; i<amostras; i++) {
    voltage[i]=leitura_v[i]*(9.0/4096.0);
    dtostrf(voltage[i], 1, 2, voltString);
    Serial.println(voltage[i]);
    client.publish(voltage_topic, voltString);
    delay(100);
  }
  for (int i=0; i<amostras; i++) {
    raw_volt = leitura_i[i]*(3.3/4096.0);
    current[i]= (1.65-raw_volt)*1000/0.185;
    dtostrf(current[i], 1, 2, currString);
    Serial.println(leitura_i[i]);
    Serial.println(current[i]);
    client.publish(current_topic, currString);
    delay(100);
  }  
}



    
  
