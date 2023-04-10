import os
import socket
import subprocess

class NetInterface():
    def __init__(self, type : int, ip_addr = '', port = '') -> None:
        '''
            type    : int -> 0 for serveur
                             1 for client
            ip_addr : str -> ip server only use for client
            port    : str -> port server only use for client

        '''
        self.type = type
        self.ip_addr = ip_addr
        self.port_srv = port
        self.c = None
        self.users = [ {'user':'', 'id': 1},
                       {'user':'', 'id':-1},]
        f = open('online/name.info', 'r')
        self.users[0]["user"] = f.read()
        f.close()
        self.buf_recv = b''
        self.port = 1024
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def get_nbjoueurs(self) :
        nb_joueur = 0
        for i in range(1,len(self.users)):
            if int(self.users[i]["id"]) > 0:
                nb_joueur += 1
        return nb_joueur

    def start(self) -> None:
        '''
            start client or server for multi play
        '''
        validPort = False
        while not validPort: # recherche un port non ouvert
            try:
                self.sock.bind(("127.0.0.1", self.port))
                validPort = True
            except PermissionError:
                self.port += 1
            except OSError:
                self.port += 1
        print(f"[python] serveur local python : 127.0.0.1:{self.port}")
        # demarrage du processuce C
        subprocess.run("make -C online clean", shell=True) # temporaire
        if not os.path.isfile("online/online"):
            subprocess.run(["make", "-C", "online"])
        if self.type == 0:
            print("[python] lancement du serveur")
            subprocess.call(f"./online/online {str(self.port)} &", shell=True)
        else:
            print("[python] lancement du client")
            print(f"./online/online {str(self.port)} {self.ip_addr} {self.port_srv} &")
            subprocess.call(f"./online/online {str(self.port)} {self.ip_addr} {self.port_srv} &", shell=True)

    def stop(self) -> None:
        '''
            stop client or server for multi play
        '''
        self.send_msg("i{d{0}}")
        self.c = None
        self.port = 1024

    def send_msg(self, msg : str) -> None:
        if self.c == None: return None
        self.sock.sendto(msg.encode(), self.c)

    def send_msg_s(self, msg : str) -> None:
        if self.c == None: 
            return None
        tmp = "s{"+msg+"}"
        self.sock.sendto(tmp.encode(), self.c)

    def get_msg(self) -> bool:
        retour = False
        while True:
            try:
                addr = self.sock.recvfrom(self.port, socket.MSG_DONTWAIT)
                if self.c == None:
                    self.c = addr[1]
                    print("[python] processus en c : ", self.c)
                if addr[1] != self.c:
                    return False
                self.buf_recv += addr[0]
                retour = True
            except BlockingIOError:
                break
        return retour
    
    def encrypter_msg(self, nom_fonction, *parametres):
        msg_encode = nom_fonction
        for i in parametres:
            msg_encode = msg_encode +":"+i
        msg_encode=msg_encode
        return msg_encode

    def decrypter_msg(self, msg_encode):
        msg_decode = msg_encode.split(":")
        nom_fonction = msg_decode[0]
        parametres = msg_decode[1:]
        return [nom_fonction, parametres]

    def translate_msg(self, val : int) -> None:
        self.buf_recv = self.buf_recv.decode()[val+1:].encode()

    '''
    #### Seter
    '''
    def set_ip_addr_srv(self, new_ip : str) -> None:
        if self.c == None:
            self.ip_addr = new_ip
    def set_port_srv(self, new_port : str) -> None:
        if self.c == None:
            self.port_srv = new_port
    def creat_srv(self) -> None:
        if self.c == None:
            self.type = 0
    def cret_clt(self) -> None:
        if self.c == None:
            self.type = 1
    
    def getUsers(self):
        return self.users
    def setUsers(self, listeUsers):
        self.users = listeUsers