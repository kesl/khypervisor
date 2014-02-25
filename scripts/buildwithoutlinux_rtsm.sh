cd ../platform-device/cortex_a15x2_rtsm/
export BUILD_ROOT=$PWD
echo "=== build bmguest ==="
cd guestos/bmguest
make clean;make
cp bmguest.bin ../../guestimages/
echo "=== build guestloader ==="
cd $BUILD_ROOT
cd guestos/guestloader
make clean;make
cp guestloader.bin ../../guestimages/guest0.bin
cp guestloader.bin ../../guestimages/guest1.bin
cd $BUILD_ROOT
echo "=== build hypervisor==="
make clean
make
RTSM_VE_Cortex-A15x1-A7x1 -a coretile.cluster0.cpu0=hvc-man-switch.axf
