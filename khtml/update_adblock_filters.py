#!/usr/bin/python2

# this scripts updates the adblock subscriptions config from upstream

import os, sys
import xml.etree.ElementTree as ET
if int(sys.version_info[0]) < 3:
    from urllib import urlopen
else:
    from urllib.request import urlopen

# see https://raw.githubusercontent.com/adblockplus/adblockplus/master/defaults/prefs.js
surl = "https://adblockplus.org/subscriptions2.xml"

print('Getting subscriptions from: %s' % surl)
sfd = urlopen(surl)
scontent = sfd.read()
sfd.close()

header = '''[Filter Settings]
Count=0
Enabled=true
HTMLFilterListMaxAgeDays=7
Shrink=false'''
subscriptions = ET.fromstring(scontent)

count = 1
print('Parsing subscriptions (main)...')
for variant in subscriptions.findall('subscription/variants/variant'):
    name = variant.attrib['title']
    url = variant.attrib['url']
    # that would be the place to enable some filters by default
    enabled = 'false'
    header += '''HTMLFilterList-Enabled-%d=%s
HTMLFilterListLocalFilename-%d=adblockfilter%d.txt
HTMLFilterListName-%d=%s
HTMLFilterListURL-%d=%s
''' % (count, enabled,
       count, count,
       count, name,
       count, url)
    print(name)
    count+=1

print('Parsing subscriptions (supplements)...')
for variant in subscriptions.findall('subscription/supplements/subscription/variants/variant'):
    name = variant.attrib['title']
    url = variant.attrib['url']
    # that would be the place to enable some filters by default
    enabled = 'false'
    header += '''HTMLFilterList-Enabled-%d=%s
HTMLFilterListLocalFilename-%d=adblockfilter%d.txt
HTMLFilterListName-%d=%s
HTMLFilterListURL-%d=%s
''' % (count, enabled,
       count, count,
       count, name,
       count, url)
    print(name)
    count+=1

print('Writing khtmlrc...')
with open('khtmlrc', 'w') as rc:
    rc.write(header)
