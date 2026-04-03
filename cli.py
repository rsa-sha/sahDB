import socket
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-H", "--host", default="127.0.0.1", help="IP of node or localhost")
parser.add_argument("-p", "--port", type=int, required=True, help="Server port")

args = parser.parse_args()

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((args.host, args.port))
s.sendall(b"USER_JOIN")

while True:
    resp = s.recv(4096)
    if not resp:
        break
    print(resp.decode(), end="", flush=True)

    cmd = input("").strip()
    s.sendall((cmd + "\n").encode())
