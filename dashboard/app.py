from flask import Flask, render_template, jsonify
import socket
import os

app = Flask(__name__)

CACHEX_HOST = os.getenv('CACHEX_HOST', 'localhost')
CACHEX_PORT = int(os.getenv('CACHEX_PORT', 6379))

def send_command(cmd):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(2)
        s.connect((CACHEX_HOST, CACHEX_PORT))
        s.sendall(f"{cmd}\n".encode())
        
        response = b""
        while True:
            chunk = s.recv(4096)
            if not chunk:
                break
            response += chunk
            if b'\n' in chunk:
                break
                
        s.close()
        return response.decode('utf-8').strip()
    except Exception as e:
        return None

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/stats')
def get_stats():
    # Get STATS
    stats_raw = send_command('STATS')
    if not stats_raw:
        return jsonify({"status": "offline"})
        
    stats_lines = stats_raw.split('\n')
    stats_dict = {}
    for line in stats_lines:
        if ':' in line:
            key, val = line.split(':')
            stats_dict[key.strip().lower()] = int(val.strip())
            
    # Get SIZE
    size_raw = send_command('SIZE')
    size = int(size_raw) if size_raw and size_raw.isdigit() else 0
    
    hits = stats_dict.get('hits', 0)
    misses = stats_dict.get('misses', 0)
    writes = stats_dict.get('writes', 0)
    total_ops = hits + misses + writes
    hit_rate = round((hits / (hits + misses) * 100) if (hits + misses) > 0 else 0, 2)
    
    return jsonify({
        "status": "online",
        "hits": hits,
        "misses": misses,
        "writes": writes,
        "total_ops": total_ops,
        "hit_rate": hit_rate,
        "size": size,
        "capacity": 1000
    })

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5050, debug=True)
