from flask import Flask, jsonify
import psycopg2

# PI0_IP = "localhost"
PI0_IP = "192.168.1.136" # Raspberry pi Zero's local ip address

app = Flask(__name__)

# Needed variables for connecting to postgre
DB_CONFIG = {
    'dbname'    : 'esp8266',
    'user'      : 'yose',
    'password'  : '101325',
    'host'      : PI0_IP,
    'port'      : 5432
}

def get_data():
    try:
        conn = psycopg2.connect(**DB_CONFIG)
        cur = conn.cursor()
        cur.execute("SELECT * FROM dht11")
        rows = cur.fetchall()

        # Get column names
        column_names = [desc[0] for desc in cur.description]

        # Transform the data to dictionary
        result = []
        for row in rows:
            row_dict = dict(zip(column_names, row))
            result.append(row_dict)

        return result
    
    except Exception as e:
        print("Veritabanı hatası:", e)
        return []
    
    finally:
        cur.close()
        conn.close()

@app.route('/data', methods=['GET'])
def data():
    rows = get_data()
    return jsonify(rows)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
