#!/bin/python3
#  -*- coding: utf-8 -*-

import socket
import time

HOST = '192.168.40.0'
PORT = 6667

data = []

with open('src_dump_.txt', 'r') as f:
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

if __name__ == '__main__':
        exit(0)
