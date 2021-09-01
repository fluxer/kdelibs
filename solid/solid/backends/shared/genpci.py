#!/usr/bin/python2

# pci.ids can be obtained from:
# https://raw.githubusercontent.com/pciutils/pciids/master/pci.ids

import sys

def splitpciline(fromline):
    doublespaceindex = fromline.index(b'  ')
    lineid = fromline[:doublespaceindex]
    linename = fromline[doublespaceindex+2:]
    linename = linename.replace('"', '\\"')
    # what is the question? (in 2a15)
    linename = linename.replace('???', 'Unknown')
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
            devicemap[deviceid] = devicename
        else:
            ingroupsection = False
            vendorid, vendorname = splitpciline(sline)
            if b' ' in vendorid:
                print('ranges are not supported: %s' % vendorid)
                sys.exit(123)
            vendormap[vendorid] = vendorname

print('''static const struct pciVendorTblData {
    const char* const vendorid;
    const char* const vendorname;
} pciVendorTbl[] = {''')
for vendorid in vendormap.keys():
    print('    { "%s", "%s" },' % (vendorid, vendormap[vendorid]))
print('};')
print('static const size_t pciVendorTblSize = sizeof(pciVendorTbl) / sizeof(pciVendorTblData);')
print('')
print('''static const struct pciDeviceTblData {
    const char* const deviceid;
    const char* const devicename;
} pciDeviceTbl[] = {''')
for deviceid in devicemap.keys():
    print('    { "%s", "%s" },' % (deviceid, devicemap[deviceid]))
print('};')
print('static const size_t pciDeviceTblSize = sizeof(pciDeviceTbl) / sizeof(pciDeviceTblData);')
