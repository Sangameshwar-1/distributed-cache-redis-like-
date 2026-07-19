import socket
import threading
import time
import sys

HOST = '127.0.0.1'
PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 6379
NUM_THREADS = 100
OPS_PER_THREAD = 1000

results = {'errors': 0, 'success': 0}
lock = threading.Lock()

def send_cmd(sock, cmd):
    sock.sendall((cmd + '\n').encode())
    return sock.recv(4096).decode().strip()

def worker(thread_id):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((HOST, PORT))
        local_errors = 0
        local_success = 0
        for i in range(OPS_PER_THREAD):
            key = f"t{thread_id}_k{i}"
            val = f"v{thread_id}_{i}"
            resp = send_cmd(sock, f"SET {key} {val}")
            if resp != 'OK':
                local_errors += 1
            else:
                local_success += 1
            resp = send_cmd(sock, f"GET {key}")
            if resp != val:
                local_errors += 1
            else:
                local_success += 1
        sock.close()
        with lock:
            results['errors'] += local_errors
            results['success'] += local_success
    except Exception as e:
        with lock:
            results['errors'] += OPS_PER_THREAD * 2
        print(f"Thread {thread_id} crashed: {e}")

def main():
    print(f"=== CacheX Concurrency Test ===")
    print(f"Threads: {NUM_THREADS} | Ops/thread: {OPS_PER_THREAD * 2} (SET+GET)")
    print(f"Total operations: {NUM_THREADS * OPS_PER_THREAD * 2}\n")

    threads = []
    start = time.time()
    for i in range(NUM_THREADS):
        t = threading.Thread(target=worker, args=(i,))
        threads.append(t)
        t.start()

    for t in threads:
        t.join()

    elapsed = time.time() - start
    total_ops = results['success'] + results['errors']
    print(f"\nCompleted in {elapsed:.2f}s")
    print(f"Throughput: {total_ops/elapsed:.0f} ops/sec")
    print(f"Success: {results['success']} | Errors: {results['errors']}")
    print(f"\n{'PASSED' if results['errors'] == 0 else 'FAILED'}")
    sys.exit(0 if results['errors'] == 0 else 1)

if __name__ == '__main__':
    main()
