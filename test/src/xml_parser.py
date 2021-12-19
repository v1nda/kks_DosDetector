#!/bin/python3
#  -*- coding: utf-8 -*-

def parse():

        src_file = 'main_prtg_dump.xml'
        dst_file = 'main_dump.txt'
        search_str1 = '<item>'
        search_str2 = '</item>'
        partition_str1 = '    <value_raw channel="Общий трафик (объем)" channelid="-1">'
        partition_str2 = '</value_raw>'
        partition_str3 = '.'

        with open(src_file, 'r') as src_dump:

                counter = 0
                counter_err = 0
                counter_zero = 0

                result = []
                
                flag = False

                for line in src_dump:


                        if line.find(search_str1) != -1:
                                counter += 1
                                flag = True
                                continue

                        if line.find(partition_str1) != -1:
                                res_line = line.partition(partition_str1)[2]
                                res_line = res_line.partition(partition_str2)[0]
                                result.append(res_line)

                                if res_line == '0.0000':
                                        counter_zero += 1

                                flag = False
                                continue

                        if line.find(search_str2) != -1 and flag:
                                res_line = 'err'
                                result.append(res_line)
                                counter_err += 1
                                flag = False

                print(str(counter) + ' values received')
                print(str(counter_err) + ' errors')
                print(str(counter_zero) + ' zeros')

        counter_fixed = 0

        for i in range(len(result)):

                if result[i] == 'err' or result[i] == '0.0000':
                        try:
                                result[i] = result[i - 1]
                                counter_fixed += 1
                        except IndexError:
                                result[i] = result[i + 1]
                                counter_fixed += 1
        
        print(str(counter_fixed) + ' fixed')

        with open(dst_file, 'w') as dst_dump:

                for value in result:
                        dst_dump.writelines(value.partition(partition_str3)[0] + '\n')


if __name__ == '__main__':
        parse()
        print('Done.')
