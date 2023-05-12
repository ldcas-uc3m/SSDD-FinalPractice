# ---
# Cliente de envio de mensajes básico con sockets TCP
# ---

import socket
import sys

sys.path.append('..')
from lib.lines import *



def main():

    # arguments
    if len(sys.argv) != 3:
        print("Usage: python3", sys.argv[0], "<serverAddress> <serverPort>")
        return
    
    # create socket
    sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # get server address
    server_addr = (sys.argv[1], int(sys.argv[2]))
    print("Connecting to", server_addr[0], "port", server_addr[1])


    # main loop
    while True:
        msg_len = int(input("Longitud del mensaje? "))
        answer_len = int(input("Longitud de la respuesta? "))

        # establish connection
        try:
            sd.connect(server_addr)
        except ConnectionRefusedError:
            print("Error en la conexión")
            return
        
        # message
        for _ in range(msg_len):
            msg = input("C> ")
            sendString(msg, sd)
        
        # answer
        for _ in range(answer_len):
            print("S>", readString(sd))
            
        sd.close()
    



if __name__ == "__main__":
    main()
    
    