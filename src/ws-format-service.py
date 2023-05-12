# ---
# Simple space erraser and conversion SOAP webservice
# ---


from spyne import Application, ServiceBase, Iterable, String, Unicode, rpc
from spyne.protocol.soap import Soap11
from spyne.server.wsgi import WsgiApplication

import re
import argparse

port = None


class FormatMSG(ServiceBase):

    @rpc(String, _returns=String)
    def format(ctx, msg):
        """
        Eliminates multiple spaces in a string
        """

        return re.sub(' +', ' ', msg.strip())




application = Application(
    services=[FormatMSG],
    tns='http://tests.python-zeep.org/',
    in_protocol=Soap11(validator='lxml'),
    out_protocol=Soap11())

application = WsgiApplication(application)


# ---
# Parser
# ---

def usage(path):
        print("Usage: python3 py -p <port>")


def parseArguments(argv):
    global port

    parser = argparse.ArgumentParser()
    parser.add_argument('-p', type=int, required=True, help='Server Port')
    args = parser.parse_args()

    if ((args.p < 1024) or (args.p > 65535)):
        parser.error("Error: Port must be in the range 1024 <= port <= 65535")
        return False

    port = args.p

    return True



if __name__ == '__main__':
    import logging

    from wsgiref.simple_server import make_server

    # parse arguments
    if (not parseArguments([])):
        usage()
        exit()

    # start server
    logging.basicConfig(level=logging.DEBUG)
    logging.getLogger('spyne.protocol.xml').setLevel(logging.DEBUG)

    server = make_server('127.0.0.1', port, application)

    logging.info("listening to http://127.0.0.1:" + str(port))
    logging.info("wsdl is at: http://localhost:" + str(port) + "/?wsdl")

    server.serve_forever()
