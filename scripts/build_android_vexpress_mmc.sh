#!/bin/bash
export APPLY_ROOT=$PWD

cd $PWD/platform-device/cortex_a15x2_rtsm/guestos/android_boot/

wget http://releases.linaro.org/13.08/android/vexpress/boot.tar.bz2
wget http://releases.linaro.org/13.08/android/vexpress/system.tar.bz2
wget http://releases.linaro.org/13.08/android/vexpress/userdata.tar.bz2
linaro-android-media-create --image-file linaro.img --image-size 2000M --dev vexpress --boot boot.tar.bz2 --system system.tar.bz2 --userdata userdata.tar.bz2
