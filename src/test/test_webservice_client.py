import zeep
import argparse


server = None
port = None



# ---
# Parser
# ---

def usage(path):
        print("Usage: python3 py -s <server> -p <port>")


def parseArguments(argv):
    global port
    global server

    parser = argparse.ArgumentParser()
    parser.add_argument('-s', type=str, required=True, help='Server IP')
    parser.add_argument('-p', type=int, required=True, help='Server Port')
    args = parser.parse_args()

    if (args.s is None):
        parser.error("Usage: python3 py -s <server> -p <port>")
        return False

    if ((args.p < 1024) or (args.p > 65535)):
        parser.error("Error: Port must be in the range 1024 <= port <= 65535")
        return False

    server = args.s
    port = args.p

    return True


def main(argv):

    # parse
    if (not parseArguments([])):
        usage()
        exit()

    wsdl_url = "http://" + server + ":" + str(port) + "/?wsdl"
    soap = zeep.Client(wsdl=wsdl_url)

    result = soap.service.format("En     el aeropuerto   hay   avione   ")
    print(result)


if __name__ == '__main__':
    main([])