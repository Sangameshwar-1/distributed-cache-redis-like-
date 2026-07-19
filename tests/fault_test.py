import socket
import time
import sys

HOST = '127.0.0.1'
PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 6379

def send_cmd(sock, cmd):
    sock.sendall((cmd + '\n').encode())
    return sock.recv(4096).decode().strip()

def test_kill_client():
    """Kill client mid-connection. Server should survive."""
    print("[TEST] Kill client mid-connection...")
    for i in range(20):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((HOST, PORT))
        send_cmd(sock, f"SET fault_key_{i} val")
        sock.close()  # Abrupt disconnect
    # Verify server is still alive
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, PORT))
    resp = send_cmd(sock, "SET alive_check 1")
    sock.close()
    assert resp == 'OK', f"Server died after client kills! Got: {resp}"
    print("  PASSED: Server survived 20 abrupt client disconnects")

def test_rapid_reconnect():
    """Rapid connect/disconnect cycles."""
    print("[TEST] Rapid connect/disconnect (100 cycles)...")
    for i in range(100):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((HOST, PORT))
        sock.close()
    # Verify server is still alive
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, PORT))
    resp = send_cmd(sock, "SET rapid_check 1")
    sock.close()
    assert resp == 'OK', f"Server died after rapid reconnects! Got: {resp}"
    print("  PASSED: Server survived 100 rapid connect/disconnect cycles")

def test_partial_command():
    """Send partial data then disconnect."""
    print("[TEST] Partial command then disconnect...")
    for i in range(10):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((HOST, PORT))
        sock.sendall(b"SET partial_ke")  # No newline, incomplete
        time.sleep(0.05)
        sock.close()
    # Verify server still alive
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, PORT))
    resp = send_cmd(sock, "SET partial_check 1")
    sock.close()
    assert resp == 'OK', f"Server died after partial commands! Got: {resp}"
    print("  PASSED: Server survived 10 partial command disconnects")

def main():
    print(f"=== CacheX Fault Injection Test ===")
    print(f"Target: {HOST}:{PORT}\n")
    test_kill_client()
    test_rapid_reconnect()
    test_partial_command()
    print(f"\nAll fault tests PASSED")

if __name__ == '__main__':
    main()
