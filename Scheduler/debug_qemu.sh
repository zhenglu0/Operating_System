#!/bin/bash

cd src
make
cd ..
sh ./update_image.sh
echo "Start gdb using 'gdb src/kernel' and then type 'target remote :1234'"
qemu -fda floppy.img -s -S &
