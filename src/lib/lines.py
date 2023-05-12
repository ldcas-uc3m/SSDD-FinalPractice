import socket


MAX_LINE = 256


class ExceededMaximumLineException(Exception):
    "Exeeded the maximum line size"
    pass


def readString(sd: socket.socket):
    # *
    # * @brief Decodes the received string from a socket
    # *
    # * @param sd    - socket
    # *
    # * @return msg  - message in string format
    
    msg = ''

    while True:
        try:
            char = sd.recv(1)
        except Exception as e:
            raise e
        
        if (char == b'\0'):
            return(msg)

        msg += char.decode()

        if len(msg) >= MAX_LINE:
            raise ExceededMaximumLineException
    


def sendString(msg: str, sd: socket.socket):
    # *
    # * @brief Sends a string message through a socket
    # *
    # * @param msg   - string to be sent
    # * @param sd    - socket
    # *

    if len(msg) > MAX_LINE - 1:
        raise ExceededMaximumLineException

    sd.sendall(msg.encode() + b'\0')
