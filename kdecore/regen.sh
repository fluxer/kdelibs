#!/bin/sh

set -e

# a little helper script to fetch PCI and USB IDs databases

rm -vf pci.ids usb.ids

wget https://raw.githubusercontent.com/pciutils/pciids/master/pci.ids
wget http://www.linux-usb.org/usb.ids
