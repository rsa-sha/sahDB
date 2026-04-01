import socket

IP = input("IP of node|localhost: ").strip()
port = int(input("Server port: ").strip())
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((IP, port))
s.sendall("USER_JOIN".encode())

while True:
    resp = s.recv(4096).decode()
    if not resp:
        break
    print(resp, end="")
    cmd = input().strip()
    s.sendall((cmd+"\n").encode())
