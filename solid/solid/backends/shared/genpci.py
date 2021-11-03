#!/usr/bin/python3

# pci.ids can be obtained from:
# https://raw.githubusercontent.com/pciutils/pciids/master/pci.ids

import sys

def isvalidid(fromid):
    if len(fromid) == 4:
        return True
    return False

def bytetostr(frombytes):
    return frombytes.decode('utf-8')

def splitpciline(fromline):
    doublespaceindex = fromline.index(b'  ')
    lineid = fromline[:doublespaceindex]
    linename = fromline[doublespaceindex+2:]
    # nested quotes
    linename = linename.replace(b'"', b'\\"')
    # what is the question?
    linename = linename.replace(b'??', b'Unknown') # 036c
    linename = linename.replace(b'???', b'Unknown') # 2a15
    return (lineid, linename)

vendormap = {}
devicemap = {}
with open('./pci.ids', 'rb') as f:
    ingroupsection = False
    for line in f.readlines():
        sline = line.strip()
        if not sline or sline.startswith(b'#'):
            continue;
        elif line.startswith(b'\t\t'):
            # subvendor
            continue
        elif line.startswith(b'C'):
            ingroupsection = True
        elif line.startswith(b'\t') and not ingroupsection:
            deviceid, devicename = splitpciline(sline)

            if b' ' in deviceid:
                print('ranges are not supported: %s' % deviceid)
                sys.exit(123)
            if not isvalidid(deviceid):
                continue

            if not vendorid in devicemap.keys():
                devicemap[vendorid] = []
            devicemap[vendorid].append({'deviceid': deviceid, 'devicename': devicename })
        else:
            ingroupsection = False
            vendorid, vendorname = splitpciline(sline)

            if b' ' in vendorid:
                print('ranges are not supported: %s' % vendorid)
                sys.exit(123)
            if not isvalidid(vendorid):
                continue

            vendormap[vendorid] = vendorname

print('''static const struct pciVendorTblData {
    const char* const vendorid;
    const char* const vendorname;
} pciVendorTbl[] = {''')
for vendorid in vendormap.keys():
    print('    { "%s", "%s" },' % (bytetostr(vendorid), bytetostr(vendormap[vendorid])))
print('};')
print('static const size_t pciVendorTblSize = sizeof(pciVendorTbl) / sizeof(pciVendorTblData);')
print('')
print('''static const struct pciDeviceTblData {
    const char* const vendorid;
    const char* const deviceid;
    const char* const devicename;
} pciDeviceTbl[] = {''')
for vendorid in devicemap.keys():
    for devicedict in devicemap[vendorid]:
        print('    { "%s", "%s", "%s" },' % (bytetostr(vendorid), bytetostr(devicedict['deviceid']), bytetostr(devicedict['devicename'])))
print('};')
print('static const size_t pciDeviceTblSize = sizeof(pciDeviceTbl) / sizeof(pciDeviceTblData);')
