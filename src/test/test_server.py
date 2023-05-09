# ---
# Servidor de echo básico con sockets TCP
# ---

import socket
import sys

sys.path.append('..')
from lib.lines import *


# ---
# CONTROLS
# ---

# length of the request
REQUEST_LEN = 4

def answer(sd):
    # answer to the request

    sendString("0", sd)
    print("S>", "0")



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
        # accept connection
        newsd, client_addr = sd.accept()
        print("Client connected")

        # receive
        for _ in range(REQUEST_LEN):
            msg = readString(newsd)
            print("C>", msg)

        # answer
        answer(newsd)

        newsd.close()


if __name__ == "__main__":
    main()