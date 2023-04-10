import ctypes


udp_client_send = ctypes.CDLL(
    "./udp_client_send.so")
udp_client_receive = ctypes.CDLL(
    "./udp_client_receive.so")

udp_client_send.send_message.argtypes = [ctypes.c_char_p]
udp_client_send.send_message.restype = None


udp_client_receive.receive_message.argtypes = []
udp_client_receive.receive_message.restype = None


def send_msg(msg):
    udp_client_send.send_message(msg.encode())


def receive_msg():
    udp_client_receive.receive_message()


while True:
    choice = input("Enter 's' to send a message or 'r' to receive a message: ")
    if choice == 's':
        send_msg("Hello")
    elif choice == 'r':
        receive_msg()
    else:
        print("Invalid choice. Please enter 's' or 'r'.")
