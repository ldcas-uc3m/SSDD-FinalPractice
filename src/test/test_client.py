# ---
# Cliente de envio de mensajes básico con sockets TCP
# ---

import socket
import sys

sys.path.append('..')
from lib.lines import *

# controls the lenght (in number of messages) of the expected answer
ANSWER_LEN = 1  


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

    # establish connection
    try:
        sd.connect(server_addr)
    except ConnectionRefusedError:
        print("Error en la conexión")
        return

    # main loop
    while True:
        try:
            msg = input()
            sendString(msg, sd)

            # if msg == "EXIT":
            #     break
            
            for _ in range(ANSWER_LEN):
                print("S>", readString(sd))
                
        except:
            break
    
    sd.close()



if __name__ == "__main__":
    main()
    
    