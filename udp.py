import socket
import sys

# get IP address and port number from command line arguments
ip_address = sys.argv[1]
port_number = int(sys.argv[2])

# create UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

while True:
    # read user input from terminal
    message = input("Enter message: ")

    # send message to udp.c client
    sock.sendto(message.encode(), (ip_address, port_number))

    # receive response from udp.c client
    data, addr = sock.recvfrom(1024)

    # print response
    print("Received response:", data.decode())
