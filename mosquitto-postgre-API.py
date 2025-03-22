import paho.mqtt.client as mqtt
import psycopg2
from psycopg2.extras import RealDictCursor
from datetime import datetime
import json

PI0_IP = "localhost"
# PI0_IP = "192.168.1.136" # Raspberry pi Zero's local ip address
mqtt_topic = "esp8266/dht11"
temp = 0
hum = 0

# Needed variables for connecting to postgre
DB_CONFIG = {
    'dbname'    : 'esp8266',
    'user'      : 'yose',
    'password'  : '101325',
    'host'      : PI0_IP,
    'port'      : 5432
}

def get_db_conn():
    try:
        conn = psycopg2.connect(**DB_CONFIG)
        return conn
    
    except Exception as e:
        return print({"Database connection error": str(e)}), 500

def push_data(temp, hum):
    conn = get_db_conn()
    cursor = conn.cursor(cursor_factory=RealDictCursor)
    
    try:
        query = "INSERT INTO dht11 (temperature, humidity, created_at) VALUES (%s, %s, %s)"
        data = (temp, hum, datetime.now())
        cursor.execute(query, data)    
        conn.commit()
        print("Data saved to database")

    except Exception as e:
        return print({"Query error": str(e)}), 500
    
    finally:
        cursor.close()
        conn.close()

# A callback function when connected to the broker
def on_connect(client, userdata, flags, rc, properties):
    if rc == 0:
        print("Connected to the Broker!")
        # Subscribing to a topic when conection is succesful
        client.subscribe(mqtt_topic)

    else:
        print(f"Conection fail! Erro code: {rc}")

# A callback function when receiving message from the broker
def on_message(client, userdata, msg):
    decoded_msg = msg.payload.decode()
    print(f"{msg.topic} konusundan mesaj alındı: {decoded_msg}")
    
    #Saving received data to database
    data = json.loads(decoded_msg)
    temp = data.get("temperature")
    hum = data.get("humidity")
    
    push_data(temp, hum)

# Creating a MQTT client object
client = mqtt.Client(callback_api_version=mqtt.CallbackAPIVersion.VERSION2)

# Binding Callback functions
client.on_connect = on_connect
client.on_message = on_message

# Connecting to the Broker
broker_address = PI0_IP
port = 1883  # MQTT portu
client.connect(broker_address, port, 60)

# Publishing messages
#client.publish("test/topic", "Hello World!")

# Listening the broker in loop
client.loop_forever()