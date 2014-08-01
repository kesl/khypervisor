#!/bin/sh


rm -rf ../linaro/arch/arm/boot/zImage

cp .config ../linaro/.config
#mv initrd initrd.cpio
cp initrd.cpio ../linaro/

cd ../linaro/

make ARCH=arm menuconfig
make -j4 ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- zImage modules
make clean
cd ../android-linaro/
cat board.dtb >> ../linaro/arch/arm/boot/zImage




