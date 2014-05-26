#for bmguest + bmguest

#export TARGET_PRODUCT="cortex_a15x2_a7x3_vexpress"
export TARGET_PRODUCT="cortex_a15x2_rtsm"

export GUEST_COUNT=2

export HYPERVISOR_BIN="hvc-man-switch.axf"
export HYPERVISOR_BUILD_SCRIPT="make clean && \
cp patch/vexpress/model.lds.S ./ && \
make"
export HYPERVISOR_CLEAN_SCRIPT="make clean"

export UBOOT_DIR="u-boot-native"
export UBOOT="u-boot.bin"
export UBOOT_BUILD_SCRIPT="cp ../patch/vexpress/vexpress_ca15_tc2_bm.h \
./include/configs/vexpress_ca15_tc2.h && \
make vexpress_ca15_tc2 CROSS_COMPILE=arm-none-eabi-"
export UBOOT_CLEAN_SCRIPT="make clean"

export BMGUEST_BIN="bmguest.bin"
export GUEST0_DIR="guestos/guestloader"
export GUEST0_BIN="guestloader.bin"
export GUEST0_BUILD_SCRIPT="cd ../bmguest/ && \
make clean && \
make && \
cp $BMGUEST_BIN ../../guestimages/ && \
cd ../../$GUEST0_DIR && \
make clean && \
make"
export GUEST0_CLEAN_SCRIPT="make clean"

export GUEST1_DIR="guestos/guestloader"
export GUEST1_BIN="guestloader.bin"
export GUEST1_BUILD_SCRIPT="cd ../bmguest/ && \
make clean && \
make && \
cp $BMGUEST_BIN ../../guestimages/ && \
cd ../../$GUEST1_DIR && \
make clean && \
make"
export GUEST1_CLEAN_SCRIPT="make clean"

export GUEST_IMAGE_DIR="guestimages"
export CI_BUILD_DIR="bmguest_bmguest"
