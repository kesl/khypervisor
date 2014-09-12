#!/bin/bash

echo "input you are sdcard name:"
read sdcardname
echo "you are sdcard is $sdcardname"


sudo linaro-android-media-create --mmc /dev/$sdcardname --dev arndale --boot boot.tar.bz2 --system system.tar.bz2 --userdata userdata.tar.bz2

sudo fdisk /dev/$sdcardname < input

#sudo dd if=../../arndale-bl1.bin of=/dev/$sdcardname bs=512 seek=1
#sudo dd if=../../u-boot-native/spl/smdk5250-spl.bin of=/dev/$sdcardname bs=512 seek=17
#sudo dd if=../../u-boot-native/u-boot.bin of=/dev/$sdcardname bs=512 seek=49
sudo dd if=./Arndale-bl1.bin of=/dev/$sdcardname bs=512 seek=1
sudo dd if=./Smdk5250-spl.bin of=/dev/$sdcardname bs=512 seek=17
sudo dd if=./U-boot.bin of=/dev/$sdcardname bs=512 seek=49

sudo dd if=../../hvc-man-switch.bin of=/dev/$sdcardname bs=512 seek=1105
sudo dd if=../../guestimages/guest0.bin of=/dev/$sdcardname bs=512 seek=1205
sudo dd if=../../guestimages/guest1.bin of=/dev/$sdcardname bs=512 seek=9215
sudo dd if=../../guestimages/zImage of=/dev/$sdcardname bs=512 seek=9235
sudo dd if=../../guestimages/bmguest.bin of=/dev/$sdcardname bs=512 seek=16335
sudo sync





