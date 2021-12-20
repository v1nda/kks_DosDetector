#!/bin/python3
#  -*- coding: utf-8 -*-

import socket
import time

HOST = '127.0.0.1'
PORT = 443

log_file = "echo_log.txt"

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        
        s.connect((HOST, PORT))

        count = 0

        with open(log_file, 'w') as log:
                while True:
                        send_time = time.time()
                        
                        s.sendall(b'Echo test message')
                        data = s.recv(1024)

                        recieve_time = time.time()

                        result = str(count) + "\tReceived: " + str(repr(data)) + "\ttime: " +  str(round(recieve_time - send_time, 4)) + "\ts"

                        print(result)
                        log.write(result + "\n")

                        count += 1
                        time.sleep(1)
