#for bmguest + bmguest

export TARGET_PRODUCT="cortex_a15x2_arndale"
export GUEST_COUNT=2

export HYPERVISOR_BIN="hvc-man-switch.bin"
export HYPERVISOR_BUILD_SCRIPT="make clean && \
make CROSS_COMPILE=arm-linux-gnueabihf-"
export HYPERVISOR_CLEAN_SCRIPT="make clean"

export UBOOT_DIR="u-boot-native"
export UBOOT="u-boot.bin"
export UBOOT_BUILD_SCRIPT="make arndale5250 CROSS_COMPILE=arm-none-eabi-"
export UBOOT_CLEAN_SCRIPT="make clean"

export BMGUEST_BIN="bmguest.bin"
export GUEST0_DIR="guestos/guestloader"
export GUEST0_BIN="guestloader.bin"
export GUEST0_BUILD_SCRIPT="cd ../bmguest/ && \
make clean && \
make GUEST_NUMBER=0 && \
cp $BMGUEST_BIN ../../guestimages/ && \
nm bmguest.axf -n > System.map && \
cp System.map ../../guestimages/ && \
cd ../../$GUEST0_DIR && \
make clean && \
make GUEST_NUMBER=0 "
export GUEST0_CLEAN_SCRIPT="make clean"

export GUEST1_DIR="guestos/guestloader"
export GUEST1_BIN="guestloader.bin"
export GUEST1_BUILD_SCRIPT="cd ../bmguest/ && \
make clean && \
make GUEST_NUMBER=1 && \
cp $BMGUEST_BIN ../../guestimages/ && \
cd ../../$GUEST1_DIR && \
make clean && \
make GUEST_NUMBER=1 MONITOR=y"
export GUEST1_CLEAN_SCRIPT="make clean"

export GUEST_IMAGE_DIR="guestimages"
export CI_BUILD_DIR="bmguest_bmguest"
