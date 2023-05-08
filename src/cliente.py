import subprocess
import sys
import PySimpleGUI as sg
from enum import Enum
import argparse
import socket
import threading

from lib.lines import *


class client:

    # ******************** TYPES *********************
    # *
    # * @brief Return codes for the protocol methods
    class RC(Enum) :
        OK = 0
        USER_ERROR = 1
        ERROR = 2
        # for connect()
        CONNECTED_ERROR = 2
        OTHER_ERROR = 3


    # ****************** ATTRIBUTES ******************
    _server = None
    _port = -1
    _quit = 0
    _username = None
    _alias = None
    _date = None
    conversation_sd = None
    conversation_thread = None
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
        server = (client._username, client._port)

        # establish connection
        try:
            sd.connect(server)
        except ConnectionRefusedError:
            print("Error en la conexión")
            return

        return sd
    

    # ******************** METHODS *******************
    # *
    # * @brief Receiving messages from the server
    # *
    # * @param sd       - client socket
    # * @param window   - client window
    def conversation(sd: socket.socket, window: sg.Window):
        sd.listen(True)

        while True:
            try:
                sd.accept()
            except:
                exit()

            # read message
            if readString(sd) != "SEND MESSAGE":
                continue
            
            alias = readString(sd)
            id = readString(sd)
            msg = readString(sd)

            window['_CLIENT_'].print("c> MESSAGE " + id + " FROM " + alias)
            window['_CLIENT_'].print("\t" + msg)
            window['_CLIENT_'].print("\tEND")



    # *
    # * @param user  - User id to register in the system
    # * @param alias - User name to register in the system
    # * @param date  - User birthdate
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user is already registered
    # * @return ERROR if another error occurred
    @staticmethod
    def register(user, alias, date, window: sg.Window):

        try:
            sd = client.socket_connect()
        except:
            window['_SERVER_'].print("s> REGISTER FAIL")
            return
        try:
            sendString("REGISTER")
            sendString(user)
            sendString(alias)
            sendString(date)

            # wait for result

            match readString(sd):
                case client.RC.OK:
                    window['_SERVER_'].print("s> REGISTER OK")
                
                case client.RC.USER_ERROR:
                    window['_SERVER_'].print("s> USERNAME IN USE")
                
                case client.RC.ERROR:
                    window['_SERVER_'].print("s> REGISTER FAIL")
                
                case _:
                    window['_SERVER_'].print("s> REGISTER FAIL")
            
            sd.close()

        except:
            sd.close()
            window['_SERVER_'].print("s> REGISTER FAIL")



    # *
    # 	 * @param alias - User name to unregister from the system
    # 	 *
    # 	 * @return OK if successful
    # 	 * @return USER_ERROR if the user does not exist
    # 	 * @return ERROR if another error occurred
    @staticmethod
    def unregister(alias, window: sg.Window):
        
        try:
            sd = client.socket_connect()
        except:
            window['_SERVER_'].print("s> UNREGISTER FAIL")
            return
        try:
            sendString("UNREGISTER")
            sendString(alias)

            # wait for result

            match readString(sd):
                case client.RC.OK:
                    window['_SERVER_'].print("s> UNREGISTER OK")
                
                case client.RC.USER_ERROR:
                    window['_SERVER_'].print("s> USER DOES NOT EXIST")
                
                case client.RC.ERROR:
                    window['_SERVER_'].print("s> REGISTER FAIL")
                
                case _:
                    window['_SERVER_'].print("s> REGISTER FAIL")
            
            sd.close()

        except:
            sd.close()
            window['_SERVER_'].print("s> REGISTER FAIL")


    # *
    # * @param alias - User name to connect to the system
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user does not exist or if it is already connected
    # * @return ERROR if another error occurred
    @staticmethod
    def connect(alias, window: sg.Window):

        if client.conversation_sd != None:
            window['_CLIENT_'].print("c> CONNECT FAIL")
            return

        # create new socket on free port
        client.conversation_sd = socket.socket()
        client.conversation_sd.bind(('', 0))  # by using 0 the system gives a random free port
        port = client.conversation_sd.getsockname()[1]

        # create thread
        client.conversation_thread = threading.Thread(target=client.conversation, args=(client.conversation_sd, window))
        client.conversation_thread.start()

        # send message
        try:
            sd = client.socket_connect()
        except:
            window['_SERVER_'].print("s> CONNECT FAIL")
            return

        try:
            sendString("CONNECT")
            sendString(alias)
            sendString(port)

            # wait for result

            match readString(sd):
                case client.RC.OK:
                    window['_SERVER_'].print("s> DISCONNECT OK")
                
                case client.RC.USER_ERROR:
                    window['_SERVER_'].print("s> CONNECT FAIL, USER DOES NOT EXIST")
                
                case client.RC.CONNECTED_ERROR:
                    window['_SERVER_'].print("s> USER ALREADY CONNECTED")
                    
                case client.RC.OTHER_ERROR:
                    window['_SERVER_'].print("s> CONNECT FAIL")
                
                case _:
                    window['_SERVER_'].print("s> CONNECT FAIL")
                    sd.close()
            
            sd.close()

        except:
            sd.close()
            window['_SERVER_'].print("s> CONNECT FAIL")

        


    # *
    # * @param user - User name to disconnect from the system
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user does not exist
    # * @return ERROR if another error occurred
    @staticmethod
    def disconnect(alias, window: sg.Window):

        if client.conversation_sd == None:
            window['_CLIENT_'].print("c> DISCONNECT FAIL")
            return

        # close socket
        # close socket (client.conversation_sd) -> raises exception -> catch it
        try:
            client.conversation_sd.close()
            client.conversation_thread.join()
        except:  # FIXME: catch exception???
            pass

        client.conversation_sd = None


        # send message
        try:
            sd = client.socket_connect()
        except:
            window['_SERVER_'].print("s> DISCONNECT FAIL")
            return
        try:
            sendString("DISCONNECT")
            sendString(alias)

            # wait for result

            match readString(sd):
                case client.RC.OK:
                    window['_SERVER_'].print("s> CONNECT OK")
                
                case client.RC.USER_ERROR:
                    window['_SERVER_'].print("s> DISCONNECT FAIL / USER DOES NOT EXIST")
                
                case client.RC.CONNECTED_ERROR:
                    window['_SERVER_'].print("s> DISCONNECT FAIL / USER NOT CONNECTED")
                    
                case client.RC.OTHER_ERROR:
                    window['_SERVER_'].print("s> DISCONNECT FAIL")
                
                case _:
                    window['_SERVER_'].print("s> DISCONNECT FAIL")
                    sd.close()
            
            sd.close()

        except:
            sd.close()
            window['_SERVER_'].print("s> DISCONNECT FAIL")



    # *
    # * @param user    - Receiver user name
    # * @param message - Message to be sent
    # *
    # * @return OK if the server had successfully delivered the message
    # * @return USER_ERROR if the user is not connected (the message is queued for delivery)
    # * @return ERROR the user does not exist or another error occurred
    @staticmethod
    def send(alias_src, alias_dst, message, window: sg.Window):
        # print("SEND " + alias_dst + " " + message)
        try:
            sd = client.socket_connect()
        except:
            window['_SERVER_'].print("s> SEND FAIL")
            return
        try:
            sendString("SEND")
            sendString(alias_src)
            sendString(alias_dst)
            sendString(message)

            # wait for result

            match readString(sd):
                case client.RC.OK:
                    window['_SERVER_'].print("s> SEND OK - MESSAGE " + client.id)
                    try:
                        id = int(readString(sd))
                    except:
                        pass
                
                case client.RC.USER_ERROR:
                    window['_SERVER_'].print("s> SEND FAIL / USER DOES NOT EXIST")
                
                case client.RC.ERROR:
                    window['_SERVER_'].print("s> SEND FAIL")
                
                case _:
                    window['_SERVER_'].print("s> SEND FAIL")
            
            # wait for ack
            try:
                if readString(sd) == "SEND MESS ACK":
                    window['_SERVER_'].print("s> SEND MESSAGE " + id + " OK")
            except:
                pass

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
    # * @return OK if the server had successfully delivered the message
    # * @return USER_ERROR if the user is not connected (the message is queued for delivery)
    # * @return ERROR the user does not exist or another error occurred
    @staticmethod
    def sendAttach(user, message, file, window: sg.Window):
        window['_SERVER_'].print("s> SENDATTACH MESSAGE OK")
        print("SEND ATTACH " + user + " " + message + " " + file)

        # TODO (?): Send Attach

        #  Write your code here
        return client.RC.ERROR

    @staticmethod
    def connectedUsers(window):

        try:
            sd = client.socket_connect()
        except:
            window['_SERVER_'].print("s> CONNECTED USERS FAIL")
            return
        try:
            sendString("CONNECTEDUSERS")

            # wait for result

            match readString(sd):
                case client.RC.OK:
                    
                    # read users

                    msg = "s> CONNECTED USERS "
                    
                    num_users = readString(sd)
                    msg += sd + " OK - "

                    for i in range(num_users):
                        if i == num_users - 1:  # last user
                            msg += readString(sd)
                        else:
                            msg += readString(sd) + ", "

                    window['_SERVER_'].print(msg)
                
                case client.RC.USER_ERROR:
                    window['_SERVER_'].print("s> CONNECTED USERS FAIL / USER IS NOT CONNECTED")
                
                case client.RC.ERROR:
                    window['_SERVER_'].print("s> CONNECTED USERS FAIL")
                
                case _:
                    window['_SERVER_'].print("s> CONNECTED USERS FAIL")

            sd.close()

        except:
            sd.close()
            window['_SERVER_'].print("s> CONNECTED USERS FAIL")

        return client.RC.ERROR


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
        print("Usage: python3 py -s <server> -p <port>")


    # *
    # * @brief Parses program execution arguments
    @staticmethod
    def parseArguments(argv):
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

        client._server = args.s
        client._port = args.p

        return True


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

            # if (values['_IN_'] == '') and (event != 'REGISTER' and event != 'CONNECTED USERS'):
            #    window['_CLIENT_'].print("c> No text inserted")
            #    continue

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
                window['_CLIENT_'].print('c> DISCONNECT ' + client._alias)
                client.disconnect(client._alias, window)


            elif (event == 'SEND'):
                window['_CLIENT_'].print('c> SEND ' + values['_INDEST_'] + " " + values['_IN_'])

                if (values['_INDEST_'] != '' and values['_IN_'] != '' and values['_INDEST_'] != 'User' and values['_IN_'] != 'Text') :
                    client.send(client._alias, values['_INDEST_'], values['_IN_'], window)
                else :
                    window['_CLIENT_'].print("Syntax error. Insert <destUser> <message>")


            elif (event == 'SENDATTACH'):

                window['_CLIENT_'].print('c> SENDATTACH ' + values['_INDEST_'] + " " + values['_IN_'] + " " + values['_FILE_'])

                if (values['_INDEST_'] != '' and values['_IN_'] != '' and values['_FILE_'] != '') :
                    client.sendAttach(values['_INDEST_'], values['_IN_'], values['_FILE_'], window)
                else :
                    window['_CLIENT_'].print("Syntax error. Insert <destUser> <message> <attachedFile>")


            elif (event == 'CONNECTED USERS'):
                window['_CLIENT_'].print("c> CONNECTEDUSERS")
                client.connectedUsers(window)



            window.Refresh()

        window.Close()


if __name__ == '__main__':
    client.main([])
    print("+++ FINISHED +++")