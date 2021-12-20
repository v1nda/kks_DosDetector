#!/bin/python3
#  -*- coding: utf-8 -*-

import random

src_file = 'training_dump.txt'
dst_file_1 = 'src_dump_1.txt'
dst_file_2 = 'src_dump_2.txt'

start_second = 230700

def generate():

        traffic_src = []
        traffic_dst_1 = []
        traffic_dst_2 = []

        random.seed()

        with open(src_file, 'r') as src_dump:
                for line in src_dump:
                        traffic_src.append(int(line))

        for src_value in traffic_src:

                temp = []
                summ = 0

                for i in range(2):
                        temp.append(random.randint(1, 5))
                        summ += temp[i]

                temp[0] /= summ
                temp[0] *= src_value
                traffic_dst_1.append(round(temp[0]))

                temp[1] /= summ
                temp[1] *= src_value
                traffic_dst_2.append(round(temp[1]))

        with open(dst_file_1, 'w') as dst_dump:
                for i in range(len(traffic_dst_1)):
                        if i > start_second:
                                dst_dump.write(str(traffic_dst_1[i]) + '\n')

        with open(dst_file_2, 'w') as dst_dump:
                for i in range(len(traffic_dst_2)):
                        if i > start_second:
                                dst_dump.write(str(traffic_dst_2[i]) + '\n')


if __name__ == '__main__':
        generate()
        print('Done.')
