#!/bin/sh
cp .config ../linaro
cp initrd.cpio ../linaro
cd ../linaro/
make ARCH=arm menuconfig
make -j4 ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- zImage modules
cd ../android-linaro
cat board.dtb >> ../linaro/arch/arm/boot/zImage
