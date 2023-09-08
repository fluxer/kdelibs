#!/usr/bin/env python

import os, sys, glob

def cstringify(s):
    r = s.replace('\\', '\\\\')
    r = r.replace('"', '\\"')
    r = r.replace('&lt;', '<')
    r = r.replace('&gt;', '>')
    return r

if len(sys.argv) < 2:
    print('usage: scoop-rules <model|layout|variant|option>')
    exit(1)

printmodel = False
printlayout = False
printvariant = False
printoption = False
if sys.argv[1] == 'model':
    printmodel = True
elif sys.argv[1] == 'layout':
    printlayout = True
elif sys.argv[1] == 'variant':
    printvariant = True
elif sys.argv[1] == 'option':
    printoption = True
else:
    print('usage: scoop-rules <model|layout|variant|option>')
    exit(1)

lstfiles = glob.glob('/usr/share/X11/xkb/rules/*.lst')
if len(lstfiles) < 1:
    print('Could not find lst files')
    exit(2)

for lstfile in lstfiles:
    with open(lstfile, 'r') as lsthandle:
        printline = False
        for lstline in lsthandle.readlines():
            if printmodel:
                strippedline = lstline.strip()
                if strippedline.startswith('! model'):
                    printline = True
                    continue
                elif len(strippedline) == 0:
                    printline = False
                if not printline:
                    continue
                splitline = strippedline.split(' ')
                splitpart0 = splitline[0].strip()
                splitpart1 = ' '.join(splitline[1:]).strip()
                print('    { "%s", I18N_NOOP("%s") },' % (splitpart0, cstringify(splitpart1)))
            elif printlayout:
                strippedline = lstline.strip()
                if strippedline.startswith('! layout'):
                    printline = True
                    continue
                elif len(strippedline) == 0:
                    printline = False
                if not printline:
                    continue
                splitline = strippedline.split(' ')
                splitpart0 = splitline[0].strip()
                splitpart1 = ' '.join(splitline[1:]).strip()
                print('    { "%s", I18N_NOOP("%s") },' % (splitpart0, cstringify(splitpart1)))
            elif printvariant:
                strippedline = lstline.strip()
                if strippedline.startswith('! variant'):
                    printline = True
                    continue
                elif len(strippedline) == 0:
                    printline = False
                if not printline:
                    continue
                splitline = strippedline.split(' ')
                splitpart0 = splitline[0].strip()
                splitpart1 = ' '.join(splitline[1:]).strip()
                splitlayout = splitpart1.split(':')[0].strip()
                splitdescription = ' '.join(splitpart1.split(':')[1:]).strip()
                print('    { "%s", "%s", I18N_NOOP("%s") },' % (splitlayout, cstringify(splitpart0), cstringify(splitdescription)))
            elif printoption:
                strippedline = lstline.strip()
                if strippedline.startswith('! option'):
                    printline = True
                    continue
                elif len(strippedline) == 0:
                    printline = False
                if not printline:
                    continue
                splitline = strippedline.split(' ')
                splitpart0 = splitline[0].strip()
                splitpart1 = ' '.join(splitline[1:]).strip()
                print('    { "%s", I18N_NOOP("%s") },' % (splitpart0, cstringify(splitpart1)))
                