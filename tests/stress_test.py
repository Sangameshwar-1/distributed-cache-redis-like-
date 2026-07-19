import socket
import time
import sys

HOST = '127.0.0.1'
PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 6379
NUM_OPS = 100000

def send_cmd(sock, cmd):
    sock.sendall((cmd + '\n').encode())
    return sock.recv(4096).decode().strip()

def main():
    print(f"=== CacheX Stress Test ===")
    print(f"Target: {HOST}:{PORT}")
    print(f"Operations: {NUM_OPS} SETs + {NUM_OPS} GETs\n")

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, PORT))

    # Phase 1: Write
    errors = 0
    start = time.time()
    for i in range(NUM_OPS):
        resp = send_cmd(sock, f"SET key{i} value{i}")
        if resp != 'OK':
            errors += 1
    write_time = time.time() - start
    print(f"[WRITE] {NUM_OPS} SETs in {write_time:.2f}s ({NUM_OPS/write_time:.0f} ops/sec) | Errors: {errors}")

    # Phase 2: Read
    read_errors = 0
    mismatch = 0
    start = time.time()
    for i in range(NUM_OPS):
        resp = send_cmd(sock, f"GET key{i}")
        if resp == '(nil)':
            read_errors += 1
        elif resp != f"value{i}":
            mismatch += 1
    read_time = time.time() - start
    print(f"[READ]  {NUM_OPS} GETs in {read_time:.2f}s ({NUM_OPS/read_time:.0f} ops/sec) | Missing: {read_errors} | Mismatch: {mismatch}")

    sock.close()
    total_errors = errors + read_errors + mismatch
    print(f"\n{'PASSED' if total_errors == 0 else 'FAILED'}: {total_errors} total errors")
    sys.exit(0 if total_errors == 0 else 1)

if __name__ == '__main__':
    main()
