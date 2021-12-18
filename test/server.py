#!/bin/python3
# -*- coding: utf-8 -*-

import socket

HOST = ''
PORT = 6667

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
       
        s.bind((HOST, PORT))

        s.listen(1)
       
        conn, addr = s.accept()

        with conn:
                
                while True:
                        data = conn.recv(1024)

                        if not data:
                                break



if __name__ == '__main__':
        exit(0)
