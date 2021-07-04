#!/usr/bin/python2

# usb.ids can be obtained from:
# http://www.linux-usb.org/usb.ids

import sys

def splitusbline(fromline):
    doublespaceindex = fromline.index(b'  ')
    lineid = fromline[:doublespaceindex]
    linename = fromline[doublespaceindex+2:]
    linename = linename.replace('"', '\\"')
    # what is the question? (in 1183)
    linename = linename.replace('??', 'Unknown')
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
            devicemap[deviceid] = devicename
        else:
            ingroupsection = False
            vendorid, vendorname = splitusbline(sline)
            if b' ' in vendorid:
                print('ranges are not supported: %s' % vendorid)
                sys.exit(123)
            vendormap[vendorid] = vendorname

print('''static const struct usbVendorTblData {
    const char* const vendorid;
    const char* const vendorname;
} usbVendorTbl[] = {''')
for vendorid in vendormap.keys():
    print('    { "%s", "%s" },' % (vendorid, vendormap[vendorid]))
print('};')
print('static const size_t usbVendorTblSize = sizeof(usbVendorTbl) / sizeof(usbVendorTblData);')
print('')
print('''static const struct usbDeviceTblData {
    const char* const deviceid;
    const char* const devicename;
} usbDeviceTbl[] = {''')
for deviceid in devicemap.keys():
    print('    { "%s", "%s" },' % (deviceid, devicemap[deviceid]))
print('};')
print('static const size_t usbDeviceTblSize = sizeof(usbDeviceTbl) / sizeof(usbDeviceTblData);')
