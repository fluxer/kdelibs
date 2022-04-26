#!/usr/bin/python3

# usb.ids can be obtained from:
# http://www.linux-usb.org/usb.ids

import sys

def isvalidid(fromid):
    if len(fromid) == 4:
        return True
    return False

def bytetostr(frombytes):
    return frombytes.decode('utf-8')

def splitusbline(fromline):
    doublespaceindex = fromline.index(b'  ')
    lineid = fromline[:doublespaceindex]
    linename = fromline[doublespaceindex+2:]
    # invalid escape sequence in C++
    linename = linename.replace(b'\\', b'/') # 1400
    # nested quotes
    linename = linename.replace(b'"', b'\\"')
    # what is the question?
    linename = linename.replace(b'??', b'Unknown') # 1183
    return (lineid, linename)

vendormap = {}
devicemap = {}
with open('./usb.ids', 'rb') as f:
    ingroupsection = False
    for line in f.readlines():
        sline = line.strip()
        if not sline or sline.startswith(b'#'):
            continue;
        elif line.startswith(b'\t\t'):
            # subvendor
            continue
        elif line.startswith((b'C', b'AT', b'HID', b'R', b'BIAS', b'PHY', b'HUT', b'L', b'HCC', b'VT')):
            ingroupsection = True
        elif line.startswith(b'\t') and not ingroupsection:
            deviceid, devicename = splitusbline(sline)

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
            vendorid, vendorname = splitusbline(sline)

            if b' ' in vendorid:
                print('ranges are not supported: %s' % vendorid)
                sys.exit(123)
            if not isvalidid(vendorid):
                continue

            vendormap[vendorid] = vendorname

print('''static const struct usbVendorTblData {
    const char* const vendorid;
    const char* const vendorname;
} usbVendorTbl[] = {''')
for vendorid in vendormap.keys():
    print('    { "%s", "%s" },' % (bytetostr(vendorid), bytetostr(vendormap[vendorid])))
print('};')
print('static const size_t usbVendorTblSize = sizeof(usbVendorTbl) / sizeof(usbVendorTblData);')
print('')
print('''static const struct usbDeviceTblData {
    const char* const vendorid;
    const char* const deviceid;
    const char* const devicename;
} usbDeviceTbl[] = {''')
for vendorid in devicemap.keys():
    for devicedict in devicemap[vendorid]:
        print('    { "%s", "%s", "%s" },' % (bytetostr(vendorid), bytetostr(devicedict['deviceid']), bytetostr(devicedict['devicename'])))
print('};')
print('static const size_t usbDeviceTblSize = sizeof(usbDeviceTbl) / sizeof(usbDeviceTblData);')
