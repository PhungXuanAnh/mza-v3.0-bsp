#!/bin/bash

if [[ -z $1 ]]
then
	echo "mkmmcscript Usage:"
	echo "sudo mkmmcscript <device>"
	echo "	Example:sudo mkmmcscript /dev/sdc"
	exit
fi

chmod a+x mkmmc-android.sh
./mkmmc-android.sh $1 boot/MLO boot/u-boot.bin boot/uImage boot/boot.scr rootfs_am37x.tar.bz2



