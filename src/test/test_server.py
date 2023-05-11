# ---
# Servidor simple para pruebas
# ---

import socket
import sys
import time

sys.path.append('..')
from lib.lines import *



def generic(sd):
    newsd, client_addr = sd.accept()
    print("Client connected")

    # receive
    REQUEST_LEN = 2
    for _ in range(REQUEST_LEN):
        msg = readString(newsd)
        print("C>", msg)

    # answer
    try:
        sendString("0", newsd)
        print("S>", "0")
    except:
        print("Error de conexión")



def register(sd):

    newsd, client_addr = sd.accept()
    print("Client connected")

    # receive
    REQUEST_LEN = 4
    for _ in range(REQUEST_LEN):
        msg = readString(newsd)
        print("C>", msg)

    # answer
    try:
        sendString("0", newsd)
        print("S>", "0")
    except:
        print("Error de conexión")



def connect(sd):
    # accept connection
    newsd, client_addr = sd.accept()
    print("Client connected")

    # receive
    REQUEST_LEN = 1
    for _ in range(REQUEST_LEN):
        msg = readString(newsd)
        print("C>", msg)

    alias = readString(newsd)
    print("C>", alias)
    port = readString(newsd)
    print("C>", port)


    client_sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        client_sd.connect((client_addr[0], int(port)))
    except ConnectionRefusedError:
        print("Error en la conexión")
        return
    
    # answer
    try:
        sendString("0", newsd)
        print("S>", "0")
        

        time.sleep(1)

        # test receiving messages

        sendString("SEND MESSAGE", client_sd)
        print("S>", "SEND MESSAGE")
        sendString("pepe", client_sd)
        print("S>", "pepe")
        sendString(alias, client_sd)
        print("S>", alias)
        sendString("En el aeropuerto hay avione", client_sd)
        print("S>", "En el aeropuerto hay avione")
    except:
        print("Error de conexión")

    client_sd.close()


def disconnect(sd):
    newsd, client_addr = sd.accept()
    print("Client connected")

    # receive
    REQUEST_LEN = 2
    for _ in range(REQUEST_LEN):
        msg = readString(newsd)
        print("C>", msg)

    # answer
    try:
        sendString("0", newsd)
        print("S>", "0")
    except:
        print("Error de conexión")


def send_good(sd):
    newsd, client_addr = sd.accept()
    print("Client connected")

    # receive
    REQUEST_LEN = 4
    for _ in range(REQUEST_LEN):
        msg = readString(newsd)
        print("C>", msg)

    # answer
    try:
        sendString("0", newsd)
        print("S>", "0")
        sendString("69", newsd)
        print("S>", "69")
        
        time.sleep(1)

        # ack
        sendString("SEND MESS ACK", newsd)
        print("S>", "SEND MESS ACK")
        sendString("69", newsd)
        print("S>", "69")
    except:
        print("Error de conexión")

    
def send_bad(sd):
    newsd, client_addr = sd.accept()
    print("Client connected")

    # receive
    REQUEST_LEN = 4
    for _ in range(REQUEST_LEN):
        msg = readString(newsd)
        print("C>", msg)

    # answer
    try:
        sendString("0", newsd)
        print("S>", "0")
        sendString("69", newsd)
        print("S>", "69")
        
        # no ack
    except:
        print("Error de conexión")


def connected_users(sd):
    newsd, client_addr = sd.accept()
    print("Client connected")

    # receive
    REQUEST_LEN = 1
    for _ in range(REQUEST_LEN):
        msg = readString(newsd)
        print("C>", msg)

    # answer
    try:
        sendString("0", newsd)
        print("S>", "0")

        sendString("2", newsd)
        print("S>", "2")
        sendString("chincheto77", newsd)
        print("S>", "chincheto77")
        sendString("tonacho", newsd)
        print("S>", "tonacho")
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

    register(sd)
    # connect(sd)
    # send_good(sd)
    # send_bad(sd)
    # disconnect(sd)
    connected_users(sd)
    # generic(sd)

    sd.close()


if __name__ == "__main__":
    main()