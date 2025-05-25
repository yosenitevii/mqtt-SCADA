from flask import Flask, jsonify, request, render_template
import psycopg2
from datetime import datetime

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

def get_data_by_date(date_str):
    try:
        conn = psycopg2.connect(**DB_CONFIG)
        cur = conn.cursor()
        cur.execute("SELECT * FROM dht11 WHERE DATE(created_at) = %s ORDER BY created_at ASC", (date_str,))
        rows = cur.fetchall()

        # Get column names
        column_names = [desc[0] for desc in cur.description]
        
        result = []
        for row in rows:
            row_dict = dict(zip(column_names, row))

            # formatted timestamp
            if 'created_at' in row_dict and isinstance(row_dict['created_at'], datetime):
                row_dict['created_at'] = row_dict['created_at'].isoformat()

            result.append(row_dict)
        
        return result
        
    except Exception as e:
        print("Database error:", e)
        return []
    
    finally:
        cur.close()
        conn.close()

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/data')
def data():
    date_str = request.args.get('date')

    if not date_str:
        # Default for today's date
        date_str = datetime.now().strftime('%Y-%m-%d')

    data = get_data_by_date(date_str)
    return jsonify(data)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
