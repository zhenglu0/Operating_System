#!/bin/bash

cd src && make && cd .. && sh ./update_image.sh && qemu -fda floppy.img -no-kvm
