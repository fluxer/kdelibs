#!/usr/bin/python2

# a script to check for redundant fancy headers

import os, re, sys

oregex = re.compile('#(?:[\\s]+)?include [<|"](.*)[>|"]')

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
        for smatch in oregex.findall(content):
            if smatch in lexceptions:
                continue
            if not os.path.basename(smatch) in lall:
                print(sfull, smatch)
                if '--remove' in sys.argv:
                    os.unlink(sfull)
