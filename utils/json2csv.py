#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Преобразование json-данных статусов в csv файл для последующего анализа
#
# Простой сбор данных в консоли:
# $ while :; do curl -s -m 5 'http://192.168.1.2/status' >> data.json; sleep 15; done
#
# Преобразование данных в csv:
# $ ./json2csv.py data.json 1>data.csv 2>data.err
#

import os, sys, json, time, dateutil.parser

if len(sys.argv) != 2:
    print('Usage:')
    print('     {0} data.json 1>data.csv 2>data.err'.format(sys.argv[0]))
    sys.exit(1)

sensors = [
    { 'sensor': 'SHT3X',    'type': 'ambient',  },
    { 'sensor': 'BME280',   'type': 'ambient',  },
    { 'sensor': 'MLX90614', 'type': 'ambient',  },
    { 'sensor': 'DS18B20',  'type': 'water',   'filtered': True },
    { 'sensor': 'MLX90614', 'type': 'water',   'filtered': True },
    { 'sensor': 'SHT3X',    'type': 'humidity', },
    { 'sensor': 'BME280',   'type': 'humidity', },
    { 'sensor': 'BME280',   'type': 'pressure', },
]

h = "ts\tyear\tmonth\tday\thour\tminute\tsecond";
for s in sensors:
    h += "\t{0}-{1}".format(s['sensor'], s['type'])
    if 'filtered' in s and s['filtered'] == True:
        h += "\t{0}-{1}-filtered".format(s['sensor'], s['type'])

h += "\t{0}".format('speed')
h += "\t{0}".format('uptime')

print(h)

with os.popen('cat {0}'.format(sys.argv[1])) as f:
    line = f.readline().strip()
    while line != '':
        try:
            data   = json.loads(line)
            cnt    = 0
            result = []

            for s in sensors:
                for d in data['sensors']:
                    if s['sensor'] == d['sensor'] and s['type'] == d['type']:
                        cnt += 1
                        result.append(d['value'])
                        if 'filtered' in s and s['filtered'] == True:
                            result.append(d['filtered'])
                        break

            if cnt != len(sensors) or not 'ts' in data or not 'fans' in data or not 'speed' in data['fans']  or not 'uptime' in data['fans']:
                raise Exception('Not completed json')

            t = dateutil.parser.parse(data['ts']).astimezone(dateutil.tz.tzlocal())
            s = "{0}\t{1}\t{2}\t{3}\t{4}\t{5}\t{6}".format(int(time.mktime(t.timetuple())), t.year, t.month, t.day, t.hour, t.minute, t.second)

            for v in result:
                if v == None:
                    raise Exception('None value')
                s += "\t{0}".format(v)

            s += "\t{0}".format(data['fans']['speed'])
            s += "\t{0}".format(data['fans']['uptime'])

            print(s)

        except Exception as e:
            sys.stderr.write("error line: {0}\n".format(line))
            pass

        line = f.readline().strip()
