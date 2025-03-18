#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// MQTT Broker bilgileri
const char* mqtt_server = "192.168.1.136"; // Broker adresi
const int mqtt_port = 1883; // MQTT portu
const char* mqtt_user = "ESP8266"; // Broker kullanıcı adı
const char* mqtt_password = NULL; // Broker şifresi

const char* mqtt_topic = "test_topic"; // Test konusu konu
const char* mqtt_topic_temperature = "esp8266/dht11/temperature"; // Sıcaklık konusu
const char* mqtt_topic_humidity = "esp8266/dht11/humidity"; // Nem konusu

// Her sensör için ayrı zaman damgası (lastMsg)
unsigned long lastTemp = 0;
unsigned long lastHum = 0;

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

void pub_msg(const char* mqtt_topic, char* msg, int interval, unsigned long &lastMsg){
    unsigned long now = millis();
    if (now - lastMsg > interval) {
        lastMsg = now;
        Serial.print(mqtt_topic);
        Serial.print(" ");
        Serial.println(msg);
        client.publish(mqtt_topic, msg);
    }
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

  char tempString[8];
  char humString[8];
  dtostrf(temperature, 6, 2, tempString); // Float'ı string'e çevir
  dtostrf(humidity, 6, 2, humString);     // Float'ı string'e çevir

  pub_msg(mqtt_topic_temperature, tempString, 15000, lastTemp);
  pub_msg(mqtt_topic_humidity, humString, 15000, lastHum);
}