import PySimpleGUI as sg
import argparse
import socket
import threading
import zeep

from lib.lines import *



class client:

    # ****************** ATTRIBUTES ******************
    _server = None
    _port = -1
    ws_server = None
    ws_port = -1
    _quit = 0
    _username = None
    _alias = None
    _date = None
    listen_sd = None
    listen_thread = None
    id = 0


    # ******************** METHODS *******************

    # *
    # * @brief Connects to the server and creates a socket
    # *
    # * @return sd  - Socket descriptor
    def socket_connect():

        # create socket
        sd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # get server address
        server = (client._server, client._port)

        # establish connection
        try:
            sd.connect(server)
        except Exception as e:
            print("Error en la conexión")
            raise e

        return sd
    

    # *
    # * @brief Formats the message using the format webservice
    # *
    # * @param msg - message to format
    # *
    # * @return formatted message
    def format_message(msg: str) -> str:
        wsdl_url = "http://" + client.ws_server + ":" + str(client.ws_port) + "/?wsdl"
        try:
            soap = zeep.Client(wsdl=wsdl_url)
            formatted_msg = soap.service.format(msg)
        
        except Exception as e:
            print("Error en la conexión al webservice")
            raise e
        
        return formatted_msg


    # *
    # * @brief Receiving messages from the server
    # *
    # * @param sd       - client socket
    # * @param window   - client window
    def listen(sd: socket.socket, window: sg.Window):
        sd.listen(True)

        while True:
            try:
                client.listen_sd = sd.accept()[0]
            

                # read message
                op = readString(client.listen_sd)
                if op not in ("SEND MESSAGE", "SEND MESS ACK"):
                    continue

                if op == "SEND MESS ACK":
                    id = readString(client.listen_sd)
                    window['_SERVER_'].print("s> SEND MESSAGE " + id + " OK")

                
                sender = readString(client.listen_sd)
                receiver = readString(client.listen_sd)
                msg = readString(client.listen_sd)

                window['_SERVER_'].print("s> MESSAGE " + receiver + " FROM " + sender)
                window['_SERVER_'].print("   " + msg)
                window['_SERVER_'].print("   END")

            except:
                exit()


    # *
    # * @param user  - User id to register in the system
    # * @param alias - User name to register in the system
    # * @param date  - User birthdate
    # *
    @staticmethod
    def register(user, alias, date, window: sg.Window):

        try:
            sd = client.socket_connect()
        except:
            window['_SERVER_'].print("s> REGISTER FAIL")
            return
        try:
            sendString("REGISTER", sd)
            sendString(user, sd)
            sendString(alias, sd)
            sendString(date, sd)

            # wait for result


            match int(readString(sd)):
                # yeah, it's FUCKING hardcoded, because FUCK enums and matches
                case 0:
                    window['_SERVER_'].print("s> REGISTER OK")
                
                case 1:
                    window['_SERVER_'].print("s> USERNAME IN USE")
                
                case 2:
                    window['_SERVER_'].print("s> REGISTER FAIL")
                
                case _:
                    window['_SERVER_'].print("s> REGISTER FAIL")
            
            sd.close()

        except:
            sd.close()
            window['_SERVER_'].print("s> REGISTER FAIL")


    # *
    # 	 * @param alias  - User name to unregister from the system
    # 	 * @param window - client window
    # 	 *
    @staticmethod
    def unregister(alias, window: sg.Window):
        
        try:
            sd = client.socket_connect()
        except:
            window['_SERVER_'].print("s> UNREGISTER FAIL")
            return
        try:
            sendString("UNREGISTER", sd)
            sendString(alias, sd)

            # wait for result

            match int(readString(sd)):
                case 0:
                    window['_SERVER_'].print("s> UNREGISTER OK")
                    client._alias = None
                
                case 1:
                    window['_SERVER_'].print("s> USER DOES NOT EXIST")
                
                case 2:
                    window['_SERVER_'].print("s> UNREGISTER FAIL")
                
                case _:
                    window['_SERVER_'].print("s> UNREGISTER FAIL")
            
            sd.close()

        except:
            sd.close()
            window['_SERVER_'].print("s> UNREGISTER FAIL")


    # *
    # * @param alias  - User name to connect to the system
    # * @param window - client window
    # *
    @staticmethod
    def connect(alias, window: sg.Window):

        if client.listen_sd != None:
            window['_CLIENT_'].print("c> CONNECT FAIL")
            return

        # create new socket on free port
        client.listen_sd = socket.socket()
        client.listen_sd.bind(('', 0))  # by using 0 the system gives a random free port
        port = client.listen_sd.getsockname()[1]

        # create thread
        client.listen_thread = threading.Thread(target=client.listen, args=(client.listen_sd, window))
        client.listen_thread.start()

        # send message
        try:
            sd = client.socket_connect()
        except:
            window['_SERVER_'].print("s> CONNECT FAIL")
            return

        try:
            sendString("CONNECT", sd)
            sendString(alias, sd)
            sendString(str(port), sd)

            # wait for result

            match int(readString(sd)):
                case 0:
                    window['_SERVER_'].print("s> CONNECT OK")
                
                case 1:
                    window['_SERVER_'].print("s> CONNECT FAIL, USER DOES NOT EXIST")
                
                case 2:
                    window['_SERVER_'].print("s> USER ALREADY CONNECTED")
                    
                case 3:
                    window['_SERVER_'].print("s> CONNECT FAIL")
                
                case _:
                    window['_SERVER_'].print("s> CONNECT FAIL")
                    sd.close()
            
            sd.close()

        except Exception as e:
            print(e)
            sd.close()
            window['_SERVER_'].print("s> CONNECT FAIL")
        

    # *
    # * @param user - User name to disconnect from the system
    # * @param window - client window
    # *
    @staticmethod
    def disconnect(alias, window: sg.Window):

        if client.listen_sd == None:
            window['_CLIENT_'].print("c> DISCONNECT FAIL")
            return

        # close socket
        try:
            client.listen_sd.close()
        except Exception as e:
            print(e)

        client.listen_sd = None


        # send message
        try:
            sd = client.socket_connect()
        except:
            window['_SERVER_'].print("s> DISCONNECT FAIL")
            return
        try:
            sendString("DISCONNECT", sd)
            sendString(alias, sd)

            # wait for result
            match int(readString(sd)):
                case 0:
                    window['_SERVER_'].print("s> DISCONNECT OK")
                
                case 1:
                    window['_SERVER_'].print("s> DISCONNECT FAIL / USER DOES NOT EXIST")
                
                case 2:
                    window['_SERVER_'].print("s> DISCONNECT FAIL / USER NOT CONNECTED")
                    
                case 3:
                    window['_SERVER_'].print("s> DISCONNECT FAIL")
                
                case _:
                    window['_SERVER_'].print("s> DISCONNECT FAIL")
                    sd.close()
            
            sd.close()

        except:
            sd.close()
            window['_SERVER_'].print("s> DISCONNECT FAIL")


    # *
    # * @param alias_src - Sender user name
    # * @param alias_dst - Receiver user name
    # * @param message   - Message to be sent
    # * @param window    - user window
    # *
    @staticmethod
    def send(alias_src, alias_dst, message, window: sg.Window):
        # print("SEND " + alias_dst + " " + message)
        try:
            sd = client.socket_connect()
        except:
            window['_SERVER_'].print("s> SEND FAIL")
            return
        try:

            # format msg
            formatted_msg = client.format_message(message)

            # send
            sendString("SEND", sd)
            sendString(alias_src, sd)
            sendString(alias_dst, sd)
            sendString(formatted_msg, sd)
            
            # result
            match int(readString(sd)):
                case 0:
                    # get message id
                    id = readString(sd)
                    window['_SERVER_'].print("s> SEND OK - MESSAGE " + id)

                case 1:
                    window['_SERVER_'].print("s> SEND FAIL / USER DOES NOT EXIST")
                    sd.close()
                    return
                
                case 2:
                    window['_SERVER_'].print("s> SEND FAIL")
                    
                    sd.close()
                    return
                
                case _:
                    window['_SERVER_'].print("s> SEND FAIL")
                    sd.close()
                    return
                
            sd.close()
            client.id += 1

        except:
            sd.close()
            window['_SERVER_'].print("s> SEND FAIL")
        

    # *
    # * @param user    - Receiver user name
    # * @param message - Message to be sent
    # * @param file    - file  to be sent
    # *
    @staticmethod
    def sendAttach(user, message, file, window: sg.Window):
        window['_SERVER_'].print("s> SENDATTACH MESSAGE FAIL")
        # print("SEND ATTACH " + user + " " + message + " " + file)

        # TODO (?): Send Attach


    # * @param window - user window
    # *
    @staticmethod
    def connectedUsers(window):

        try:
            sd = client.socket_connect()
        except:
            window['_SERVER_'].print("s> CONNECTED USERS FAIL")
            return
        try:
            sendString("CONNECTEDUSERS", sd)

            # wait for result

            match int(readString(sd)):
                case 0:
                    
                    # read users

                    msg = "s> CONNECTED USERS "
                    
                    num_users = int(readString(sd))
                    msg += str(num_users) + " OK - "

                    for i in range(num_users):
                        if i == num_users - 1:  # last user
                            msg += readString(sd)
                        else:
                            msg += readString(sd) + ", "

                    window['_SERVER_'].print(msg)
                
                case 1:
                    window['_SERVER_'].print("s> CONNECTED USERS FAIL / USER IS NOT CONNECTED")
                
                case 2:
                    window['_SERVER_'].print("s> CONNECTED USERS FAIL")
                
                case _:
                    window['_SERVER_'].print("s> CONNECTED USERS FAIL")

            sd.close()

        except Exception as e:
            print(e)
            sd.close()
            window['_SERVER_'].print("s> CONNECTED USERS FAIL")

        return 2


    # *
    # * @brief registration window
    @staticmethod
    def window_register():
        layout_register = [
            [sg.Text('Ful Name:'), sg.Input('Text',key='_REGISTERNAME_', do_not_clear=True, expand_x=True)],
            [sg.Text('Alias:'), sg.Input('Text',key='_REGISTERALIAS_', do_not_clear=True, expand_x=True)],
            [
                sg.Text('Date of birth:'), sg.Input('',key='_REGISTERDATE_', do_not_clear=True, expand_x=True, disabled=True, use_readonly_for_disable=False),
                sg.CalendarButton("Select Date",close_when_date_chosen=True, target="_REGISTERDATE_", format='%d-%m-%Y',size=(10,1))
            ],
            [sg.Button('SUBMIT', button_color=('white', 'blue'))]
        ]

        layout = [[sg.Column(layout_register, element_justification='center', expand_x=True, expand_y=True)]]

        window = sg.Window("REGISTER USER", layout, modal=True)
        choice = None

        while True:
            event, values = window.read()

            if (event in (sg.WINDOW_CLOSED, "-ESCAPE-")):
                break

            if event == "SUBMIT":
                if(values['_REGISTERNAME_'] == 'Text' or values['_REGISTERNAME_'] == '' or values['_REGISTERALIAS_'] == 'Text' or values['_REGISTERALIAS_'] == '' or values['_REGISTERDATE_'] == ''):
                    sg.Popup('Registration error', title='Please fill in the fields to register.', button_type=5, auto_close=True, auto_close_duration=1)
                    continue

                client._username = values['_REGISTERNAME_']
                client._alias = values['_REGISTERALIAS_']
                client._date = values['_REGISTERDATE_']
                break
        window.Close()



    # *
    # * @brief Prints program usage
    @staticmethod
    def usage() :
        print("Usage: python3 py -s <server IP> -p <server port> -ws <webservice IP> -ws <webservice port>")


    # *
    # * @brief Parses program execution arguments
    @staticmethod
    def parseArguments(argv):
        parser = argparse.ArgumentParser()
        parser.add_argument('-s', type=str, required=True, help='Server IP')
        parser.add_argument('-p', type=int, required=True, help='Server Port')
        parser.add_argument('-ws', type=str, required=True, help='Webservice server IP')
        parser.add_argument('-wp', type=int, required=True, help='Webservice server Port')
        args = parser.parse_args()

        if args.s is None or args.ws is None:
            parser.error("Usage: python3 py -s <server IP> -p <server port> -ws <webservice IP> -ws <webservice port>")
            return False

        if ((args.p < 1024) or (args.p > 65535)):
            parser.error("Error: Server port must be in the range 1024 <= port <= 65535")
            return False
        
        if ((args.wp < 1024) or (args.wp > 65535)):
            parser.error("Error: Webservice server port must be in the range 1024 <= port <= 65535")
            return False

        client._server = args.s
        client._port = args.p
        client.ws_server = args.ws
        client.ws_port = args.wp

        return True



    # *
    # * @brief main function
    def main(argv):

        if (not client.parseArguments(argv)):
            client.usage()
            exit()

        lay_col = [
            [
                sg.Button('REGISTER',expand_x=True, expand_y=True),
                sg.Button('UNREGISTER',expand_x=True, expand_y=True),
                sg.Button('CONNECT',expand_x=True, expand_y=True),
                sg.Button('DISCONNECT',expand_x=True, expand_y=True),
                sg.Button('CONNECTED USERS',expand_x=True, expand_y=True)
            ],
            [
                sg.Text('Dest:'),sg.Input('User',key='_INDEST_', do_not_clear=True, expand_x=True),
                sg.Text('Message:'),sg.Input('Text',key='_IN_', do_not_clear=True, expand_x=True),
                sg.Button('SEND',expand_x=True, expand_y=False)
            ],
            [
                sg.Text('Attached File:'), sg.In(key='_FILE_', do_not_clear=True, expand_x=True), sg.FileBrowse(),
                sg.Button('SENDATTACH',expand_x=True, expand_y=False)
            ],
            [
                sg.Multiline(key='_CLIENT_', disabled=True, autoscroll=True, size=(60,15), expand_x=True, expand_y=True),
                sg.Multiline(key='_SERVER_', disabled=True, autoscroll=True, size=(60,15), expand_x=True, expand_y=True)
            ],
            [sg.Button('QUIT', button_color=('white', 'red'))]
        ]


        layout = [[sg.Column(lay_col, element_justification='center', expand_x=True, expand_y=True)]]

        window = sg.Window('Messenger', layout, resizable=True, finalize=True, size=(1000,400))
        window.bind("<Escape>", "-ESCAPE-")


        while True:
            event, values = window.Read()

            if (event in (None, 'QUIT')) or (event in (sg.WINDOW_CLOSED, "-ESCAPE-")):
                sg.Popup('Closing Client APP', title='Closing', button_type=5, auto_close=True, auto_close_duration=1)
                break

            if (values['_IN_'] == '') and (event != 'REGISTER' and event != 'CONNECTED USERS'):
               window['_CLIENT_'].print("c> No text inserted")
               continue

            if (client._alias == None or client._username == None or client._alias == 'Text' or client._username == 'Text' or client._date == None) and (event != 'REGISTER'):
                sg.Popup('NOT REGISTERED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
                continue

            if (event == 'REGISTER'):
                client.window_register()

                if (client._alias == None or client._username == None or client._alias == 'Text' or client._username == 'Text' or client._date == None):
                    sg.Popup('NOT REGISTERED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
                    continue

                window['_CLIENT_'].print('c> REGISTER ' + client._alias)
                client.register(client._username, client._alias, client._date, window)

            elif (event == 'UNREGISTER'):
                window['_CLIENT_'].print('c> UNREGISTER ' + client._alias)
                client.unregister(client._alias, window)


            elif (event == 'CONNECT'):
                window['_CLIENT_'].print('c> CONNECT ' + client._alias)
                client.connect(client._alias, window)


            elif (event == 'DISCONNECT'):
                if (client.listen_sd == None):
                    sg.Popup('NOT CONNECTED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
                    continue
                window['_CLIENT_'].print('c> DISCONNECT ' + client._alias)
                client.disconnect(client._alias, window)


            elif (event == 'SEND'):
                if (client.listen_sd == None):
                    sg.Popup('NOT CONNECTED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
                    continue
                
                if (values['_INDEST_'] != '' and values['_IN_'] != '' and values['_INDEST_'] != 'User' and values['_IN_'] != 'Text') :
                    window['_CLIENT_'].print('c> SEND ' + values['_INDEST_'] + ' ' + values['_IN_'])
                    client.send(client._alias, values['_INDEST_'], values['_IN_'], window)
                else :
                    window['_CLIENT_'].print("Syntax error. Insert <destUser> <message>")


            elif (event == 'SENDATTACH'):

                window['_CLIENT_'].print('c> SENDATTACH ' + values['_INDEST_'] + ' ' + values['_IN_'] + " " + values['_FILE_'])

                if (values['_INDEST_'] != '' and values['_IN_'] != '' and values['_FILE_'] != '') :
                    client.sendAttach(values['_INDEST_'], values['_IN_'], values['_FILE_'], window)
                else :
                    window['_CLIENT_'].print("Syntax error. Insert <destUser> <message> <attachedFile>")


            elif (event == 'CONNECTED USERS'):
                if (client.listen_sd == None):
                    sg.Popup('NOT CONNECTED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
                    continue
                
                window['_CLIENT_'].print("c> CONNECTEDUSERS")
                client.connectedUsers(window)



            window.Refresh()

        window.Close()



if __name__ == '__main__':
    client.main([])
    print("+++ FINISHED +++")
