import socket
import sys

HOST = '127.0.0.1'
PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 6379
passed = 0
failed = 0

def send_cmd(sock, cmd):
    sock.sendall((cmd + '\n').encode())
    return sock.recv(4096).decode().strip()

def check(name, actual, expected):
    global passed, failed
    if actual == expected:
        print(f"  [PASS] {name}")
        passed += 1
    else:
        print(f"  [FAIL] {name}: expected '{expected}', got '{actual}'")
        failed += 1

def main():
    print(f"=== CacheX Integration Test ===")
    print(f"Target: {HOST}:{PORT}\n")

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, PORT))

    # SET
    print("[SET Tests]")
    check("SET name Alice", send_cmd(sock, "SET name Alice"), "OK")
    check("SET age 25", send_cmd(sock, "SET age 25"), "OK")
    check("SET city NYC", send_cmd(sock, "SET city NYC"), "OK")

    # GET
    print("\n[GET Tests]")
    check("GET name", send_cmd(sock, "GET name"), "Alice")
    check("GET age", send_cmd(sock, "GET age"), "25")
    check("GET unknown", send_cmd(sock, "GET unknown_key"), "(nil)")

    # EXISTS
    print("\n[EXISTS Tests]")
    check("EXISTS name", send_cmd(sock, "EXISTS name"), "1")
    check("EXISTS ghost", send_cmd(sock, "EXISTS ghost"), "0")

    # SIZE
    print("\n[SIZE Test]")
    check("SIZE", send_cmd(sock, "SIZE"), "3")

    # DELETE
    print("\n[DELETE Tests]")
    check("DEL name", send_cmd(sock, "DEL name"), "OK")
    check("GET deleted", send_cmd(sock, "GET name"), "(nil)")
    check("EXISTS deleted", send_cmd(sock, "EXISTS name"), "0")
    check("SIZE after del", send_cmd(sock, "SIZE"), "2")

    # OVERWRITE
    print("\n[OVERWRITE Test]")
    check("SET city LA", send_cmd(sock, "SET city LA"), "OK")
    check("GET overwritten", send_cmd(sock, "GET city"), "LA")

    # CLEAR
    print("\n[CLEAR Tests]")
    check("CLEAR", send_cmd(sock, "CLEAR"), "OK")
    check("SIZE after clear", send_cmd(sock, "SIZE"), "0")
    check("GET after clear", send_cmd(sock, "GET age"), "(nil)")

    # UNKNOWN COMMAND
    print("\n[UNKNOWN Command Test]")
    check("UNKNOWN", send_cmd(sock, "FOOBAR"), "Unknown command")

    sock.close()

    print(f"\n{'='*50}")
    print(f"Results: {passed} passed, {failed} failed")
    print(f"{'ALL TESTS PASSED' if failed == 0 else 'SOME TESTS FAILED'}")
    sys.exit(0 if failed == 0 else 1)

if __name__ == '__main__':
    main()
