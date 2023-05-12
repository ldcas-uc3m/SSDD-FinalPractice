# ---
# Servidor simple para pruebas
# ---

import socket
import sys
import time

sys.path.append('..')
from lib.lines import *

client_addr = None

def generic(sd):

    # receive
    REQUEST_LEN = 2
    for _ in range(REQUEST_LEN):
        msg = readString(sd)
        print("c>", msg)

    # answer
    try:
        sendString("0", sd)
        print("s>", "0")
    except:
        print("Error de conexión")



def register(sd):

    # receive
    REQUEST_LEN = 3
    for _ in range(REQUEST_LEN):
        msg = readString(sd)
        print("c>", msg)

    # answer
    try:
        sendString("0", sd)
        print("s>", "0")
    except:
        print("Error de conexión")



def connect(sd, client):
    global client_addr
    
    # receive
    alias = readString(sd)
    print("c>", alias)
    client_port = readString(sd)
    print("c>", client_port)


    client_sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_addr = (client[0], int(client_port))
    try:
        client_sd.connect(client_addr)
    except ConnectionRefusedError:
        print("Error en la conexión")
        return
    
    # answer
    try:
        sendString("0", sd)
        print("s>", "0")
        

        time.sleep(1)

        # test receiving messages

        sendString("SEND MESSAGE", client_sd)
        print("s>", "SEND MESSAGE")
        sendString("pepe", client_sd)
        print("s>", "pepe")
        sendString(alias, client_sd)
        print("s>", alias)
        sendString("En el aeropuerto hay avione", client_sd)
        print("s>", "En el aeropuerto hay avione")
    except:
        print("Error de conexión")

    client_sd.close()


def disconnect(sd):

    # receive
    REQUEST_LEN = 1
    for _ in range(REQUEST_LEN):
        msg = readString(sd)
        print("c>", msg)

    # answer
    try:
        sendString("0", sd)
        print("s>", "0")
    except:
        print("Error de conexión")


def send_good(sd):

    # receive
    REQUEST_LEN = 3
    for _ in range(REQUEST_LEN):
        msg = readString(sd)
        print("c>", msg)

    # answer
    try:
        sendString("0", sd)
        print("s>", "0")
        sendString("69", sd)
        print("s>", "69")
        
        time.sleep(1)

        # ack
        client_sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_sd.connect(client_addr)

        sendString("SEND MESS ACK", client_sd)
        print("s>", "SEND MESS ACK")
        sendString("69", client_sd)
        print("s>", "69")
    except:
        print("Error de conexión")

    
def send_bad(sd):

    # receive
    REQUEST_LEN = 3
    for _ in range(REQUEST_LEN):
        msg = readString(sd)
        print("c>", msg)

    # answer
    try:
        sendString("0", sd)
        print("s>", "0")
        sendString("69", sd)
        print("s>", "69")
        
        # no ack
    except:
        print("Error de conexión")


def connected_users(sd):

    # answer
    try:
        sendString("0", sd)
        print("s>", "0")

        sendString("2", sd)
        print("s>", "2")
        sendString("chincheto77", sd)
        print("s>", "chincheto77")
        sendString("tonacho", sd)
        print("s>", "tonacho")
    except:
        print("Error de conexión")
    

def unregister(sd):

    # receive
    REQUEST_LEN = 1
    for _ in range(REQUEST_LEN):
        msg = readString(sd)
        print("c>", msg)

    # answer
    try:
        sendString("0", sd)
        print("s>", "0")
    except:
        print("Error de conexión")


def main():
    # arguments
    if len(sys.argv) != 2:
        print("Usage: python3", sys.argv[0], "<serverPort>")
        return
    
    # create socket
    sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # connect
    server_addr = ("localhost", int(sys.argv[1]))
    sd.bind(server_addr)
    sd.listen(True)


    # main loop
    while True:
        try:
            # accept connection
            try:
                newsd, client = sd.accept()
                print("Client connected")
            except:
                print("Error en la conexión")
                continue
            
            # treat petition
            op = readString(newsd)
            print("s>", op)

            match op:
                case "REGISTER":
                    register(newsd)
                    
                case "CONNECT":
                    connect(newsd, client)
                
                case "SEND":
                    send_good(newsd)
                    # send_bad(sd)

                case "CONNECTEDUSERS":
                    connected_users(newsd)
                case "DISCONNECT":
                    disconnect(newsd)

                case "UNREGISTER":
                    unregister(newsd)
            
            # generic(sd)

        except KeyboardInterrupt:
            break

    sd.close()


if __name__ == "__main__":
    main()