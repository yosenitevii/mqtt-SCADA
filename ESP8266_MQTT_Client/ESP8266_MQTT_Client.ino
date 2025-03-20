#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Arduino_JSON.h>

// MQTT Broker bilgileri
const char* mqtt_server = "192.168.1.136"; // Broker adresi
const int mqtt_port = 1883; // MQTT portu
const char* mqtt_user = "ESP8266"; // Broker kullanıcı adı
const char* mqtt_password = NULL; // Broker şifresi

const char* mqtt_topic = "test_topic"; // Test konusu konu
const char* mqtt_topic_dht11 = "esp8266/dht11"; // Sıcaklık konusu
const long interval = 15000; // 
unsigned long lastMsg;

// DHT11 Sensörü için pin ve tip tanımlaması
#define DHTPIN D4     // DHT11'in bağlı olduğu pin (GPIO 2)
#define DHTTYPE DHT11 // DHT11 sensör tipi

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

#include <ESP8266WiFi.h>

// Wi-Fi bilgilerinizi girin
void setup_wifi(const char* ssid, const char* password) {
    delay(10);
    Serial.println();
    Serial.print("Wi-Fi'ye bağlanılıyor: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("Wi-Fi'ye bağlandı!");
    Serial.print("IP Adresi: ");
    Serial.println(WiFi.localIP());
}

// MQTT'ye bağlandığında çağrılacak fonksiyon
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Mesaj alındı [");
    Serial.print(topic);
    Serial.print("]: ");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
}

// MQTT broker'a bağlan
void reconnect() {
    while (!client.connected()) {
      Serial.print("MQTT broker'a bağlanılıyor...");
      // Kullanıcı adı ve şifre ile bağlan
      if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
        Serial.println("bağlandı!");
        client.subscribe(mqtt_topic); // Konuya abone ol
      } else {
        Serial.print("bağlantı başarısız, rc=");
        Serial.print(client.state());
        Serial.println(" 5 saniye sonra tekrar denenecek...");
        delay(5000);
      }
    }
}

void setup() {
  Serial.begin(115200);

  // Wi-Fi bilgilerini parametre olarak ver
  const char* wifi_ssid = "N1-L3 Raketa-nositel";
  const char* wifi_password = "101325-kPa";
  setup_wifi(wifi_ssid, wifi_password);

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  dht.begin(); // DHT11 sensörünü başlat
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // DHT11'den sıcaklık ve nem verilerini oku
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Verilerin geçerli olup olmadığını kontrol et
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("DHT11'den veri okunamadı!");
    return;
  }
  
  // Saving data to json document
  JSONVar jsonDoc;
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;

  String jsonSTR = JSON.stringify(jsonDoc);
  const char* jsonBuffer = jsonSTR.c_str();

  // Sending jsonDoc to mqtt broker every 15 sec
  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;
    client.publish(mqtt_topic_dht11, jsonBuffer);
    Serial.print("Json document transfered to broker");
    Serial.println("Mesarument data: ");
    Serial.print(jsonBuffer);
  }
}
