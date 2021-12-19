#!/bin/python3
#  -*- coding: utf-8 -*-

import random

def generate():

        src_file = 'main_dump.txt'
        dst_file = 'training_dump.txt'

        traffic_src = []
        traffic_dst = []

        random.seed()

        with open(src_file, 'r') as src_dump:
                for line in src_dump:
                        traffic_src.append(int(line))

        max_src = 0
        
        for src_value in traffic_src:

                temp = []
                summ = 0

                for i in range(15):
                        temp.append(random.randint(1, 5))
                        summ += temp[i]
                
                o = 0
                for value in temp:
                        value /= summ
                        value *= src_value
                        o += value
                        traffic_dst.append(round(value))
                # traffic_dst.append(round(o))

                if src_value > max_src:
                        max_src = src_value
        
        print(str(max_src))

        days = round(len(traffic_dst) / (60 * 60 * 24))
        print(str(days) + ' days.')

        days_max = []
        days_max_index = []
        for i in range(days):
                traffic_day = traffic_dst[round(len(traffic_dst) / days) * i:round(len(traffic_dst) / days) * (i + 1)]

                days_max.append(0)
                days_max_index.append(0)
                for j in range(len(traffic_day)):
                        if traffic_day[j] > days_max[i]:
                                days_max[i] = traffic_day[j]
                                days_max_index[i] = j + ((len(traffic_dst) / days) * i)
        
        for i in range(len(days_max)):
                print(str(i + 1) + ' day: ' + 'max: ' + str(days_max[i]) + ' bytes' + ' [' + str(days_max_index[i]) + ']')
                        
                
        with open(dst_file, 'w') as dst_dump:
                for i in range(len(traffic_dst)):
                        dst_dump.write(str(traffic_dst[i]) + '\n')


if __name__ == '__main__':
        generate()
        print('Done.')
