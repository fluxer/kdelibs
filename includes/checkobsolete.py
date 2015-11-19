#!/usr/bin/python2

# a script to check for redundant fancy headers

import os, re, sys

lexceptions = [
    '../../dnssd/settings.h',
    '../../plasma/version.h',
]

lall = []
for root, dirs, files in os.walk('%s/..' % os.getcwd()):
    for sfile in files:
        lall.append(os.path.basename(sfile))

lheaders = []
for root, dirs, files in os.walk(os.getcwd()):
    for sfile in files:
        sfull = '%s/%s' % (root, sfile)
        with open(sfull, 'rb') as f:
            content = f.read()
        for match in re.findall('#(?:[\\s]+)?include [<|"](.*)[>|"]', content):
            if match in lexceptions:
                continue
            if not os.path.basename(match) in lall:
                print(sfull, match)
                if '--remove' in sys.argv:
                    os.unlink(sfull)
