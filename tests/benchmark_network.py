import socket
import time
import sys
import csv
import os

HOST = '127.0.0.1'
NUM_OPS = 100000

def send_cmd(sock, cmd):
    sock.sendall((cmd + '\n').encode())
    return sock.recv(4096).decode().strip()

def benchmark_server(port, name):
    print(f"\n--- Benchmarking {name} on port {port} ---")
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((HOST, port))
    except Exception as e:
        print(f"  Could not connect to {name}: {e}")
        return None

    # Write benchmark
    start = time.time()
    for i in range(NUM_OPS):
        send_cmd(sock, f"SET benchkey{i} benchval{i}")
    write_time = time.time() - start
    writes_per_sec = NUM_OPS / write_time

    # Read benchmark
    start = time.time()
    for i in range(NUM_OPS):
        send_cmd(sock, f"GET benchkey{i}")
    read_time = time.time() - start
    reads_per_sec = NUM_OPS / read_time

    write_latency_ms = (write_time / NUM_OPS) * 1000
    read_latency_ms = (read_time / NUM_OPS) * 1000

    sock.close()

    results = {
        'name': name,
        'writes_per_sec': int(writes_per_sec),
        'reads_per_sec': int(reads_per_sec),
        'write_latency_ms': round(write_latency_ms, 4),
        'read_latency_ms': round(read_latency_ms, 4),
        'write_time_sec': round(write_time, 2),
        'read_time_sec': round(read_time, 2),
    }

    print(f"  Writes: {results['writes_per_sec']} ops/sec (avg {results['write_latency_ms']}ms)")
    print(f"  Reads:  {results['reads_per_sec']} ops/sec (avg {results['read_latency_ms']}ms)")
    return results

def main():
    cachex_port = int(sys.argv[1]) if len(sys.argv) > 1 else 6379
    redis_port = int(sys.argv[2]) if len(sys.argv) > 2 else 6380

    print(f"=== CacheX Network Benchmark ===")
    print(f"Operations: {NUM_OPS} SETs + {NUM_OPS} GETs per server")

    cachex_results = benchmark_server(cachex_port, "CacheX")
    redis_results = benchmark_server(redis_port, "Redis")

    # Print comparison table
    print(f"\n{'='*60}")
    print(f"{'Metric':<25} {'CacheX':>15} {'Redis':>15}")
    print(f"{'-'*60}")
    if cachex_results:
        cx = cachex_results
        rd = redis_results if redis_results else {'writes_per_sec': 'N/A', 'reads_per_sec': 'N/A', 'write_latency_ms': 'N/A', 'read_latency_ms': 'N/A'}
        print(f"{'Writes/sec':<25} {cx['writes_per_sec']:>15} {str(rd['writes_per_sec']):>15}")
        print(f"{'Reads/sec':<25} {cx['reads_per_sec']:>15} {str(rd['reads_per_sec']):>15}")
        print(f"{'Write latency (ms)':<25} {cx['write_latency_ms']:>15} {str(rd['write_latency_ms']):>15}")
        print(f"{'Read latency (ms)':<25} {cx['read_latency_ms']:>15} {str(rd['read_latency_ms']):>15}")
    print(f"{'='*60}")

    # Save to CSV
    csv_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'benchmarks', 'results.csv')
    with open(csv_path, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(['server', 'writes_per_sec', 'reads_per_sec', 'write_latency_ms', 'read_latency_ms'])
        if cachex_results:
            r = cachex_results
            writer.writerow([r['name'], r['writes_per_sec'], r['reads_per_sec'], r['write_latency_ms'], r['read_latency_ms']])
        if redis_results:
            r = redis_results
            writer.writerow([r['name'], r['writes_per_sec'], r['reads_per_sec'], r['write_latency_ms'], r['read_latency_ms']])
    print(f"\nResults saved to {csv_path}")

if __name__ == '__main__':
    main()
