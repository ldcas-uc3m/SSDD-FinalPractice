#!/bin/bash

SERVER_IP=localhost
SERVER_PORT=8080
WEBSERVICE_IP=localhost
WEBSERVICE_PORT=8888

cd src

# install python requirements
pip3 install -r requirements.txt

# server
make
./servidor -p $SERVER_PORT

# webservice
python3 ws-format-service.py -p $WEBSERVICE_PORT & > ws-log.log

# client
python3 cliente.py -s $SERVER_IP -p $SERVER_PORT -ws $WEBSERVICE_IP -wp $WEBSERVICE_PORT