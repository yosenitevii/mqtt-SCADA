import paho.mqtt.client as mqtt


# A callback function when connected to the broker
def on_connect(client, userdata, flags, rc, properties):
    if rc == 0:
        print("Connected to the Broker!")
        # Subscribing to a topic when conection is succesful
        client.subscribe("esp8266/dht11/temperature")
    else:
        print(f"Conection fail! Erro code: {rc}")

# A callback function when receiving message from the broker
def on_message(client, userdata, msg):
    decoded_msg = msg.payload.decode()
    print(f"{msg.topic} konusundan mesaj alındı: {decoded_msg}")
    
    #Saving received data to database

# Creating a MQTT client object
client = mqtt.Client(callback_api_version=mqtt.CallbackAPIVersion.VERSION2)

# Binding Callback functions
client.on_connect = on_connect
client.on_message = on_message

# Connecting to the Broker
broker_address = "192.168.1.136"  # Raspberry pi Zero's ip address
port = 1883  # MQTT portu
client.connect(broker_address, port, 60)

# Mesaj yayınla
#client.publish("test/topic", "Hello World!")

# Listening the broker in loop
client.loop_forever()