import socket
import sys


ip_address = sys.argv[1]
port_number = int(sys.argv[2])


sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

while True:
    message = input("Enter message: ")

    sock.sendto(message.encode(), (ip_address, port_number))

    # receive response from udp.c client
    data, addr = sock.recvfrom(1024)

    # print response
    print("Received response:", data.decode())
