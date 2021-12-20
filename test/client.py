#!/bin/python3
#  -*- coding: utf-8 -*-

import socket
import time

HOST = '127.0.0.1'
PORT = 6667

src_file = 'src_dump_1.txt'

data = []

with open(src_file, 'r') as f:
        for i in f:
                data.append(int(i))

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        
        s.connect((HOST, PORT))

        while True:
                for i in data:
                        string = ""
                        
                        for j in range(i):
                                string = string + '0'
                        
                        s.sendall(string.encode('ascii'))

                        time.sleep(1)
