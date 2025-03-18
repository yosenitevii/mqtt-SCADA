import paho.mqtt.client as mqtt


# Broker'a bağlandığında çağrılacak callback fonksiyonu
def on_connect(client, userdata, flags, rc, properties):
    if rc == 0:
        print("Broker'a başarıyla bağlandı!")
        # Bağlantı başarılı olduğunda bir konuya abone ol
        client.subscribe("esp8266/dht11/temperature")
    else:
        print(f"Bağlantı başarısız! Hata kodu: {rc}")

# Broker'dan mesaj alındığında çağrılacak callback fonksiyonu
def on_message(client, userdata, msg):
    decoded_msg = msg.payload.decode()
    print(f"{msg.topic} konusundan mesaj alındı: {decoded_msg}")
    
    return decoded_msg

# MQTT client oluştur
client = mqtt.Client(callback_api_version=mqtt.CallbackAPIVersion.VERSION2)

# Callback fonksiyonlarını bağla
client.on_connect = on_connect
client.on_message = on_message

# Broker'a bağlan
broker_address = "192.168.1.136"  # Yerel ağ ip adresi
#broker_address = "78.181.154.229"  # Public ip 
port = 1883  # MQTT portu
client.connect(broker_address, port, 60)

# Mesaj yayınla
#client.publish("test/topic", "vavilov'dan selam")

# Sonsuz döngüde mesajları dinle
client.loop_forever()