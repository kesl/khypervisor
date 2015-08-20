#for bmguest + bmguest

export TARGET_PRODUCT="cortex_a15x2_rtsm"

export GUEST_COUNT=4

export HYPERVISOR_BIN="hvc-man-switch.axf"
export HYPERVISOR_BUILD_SCRIPT="make clean && \
make"
export HYPERVISOR_CLEAN_SCRIPT="make clean"

export UBOOT_DIR=""
export UBOOT=""
export UBOOT_BUILD_SCRIPT=""
export UBOOT_CLEAN_SCRIPT=""

export BMGUEST_BIN="bmguest.bin"
export GUEST0_DIR="guestos/guestloader"
export GUEST0_BIN="guestloader.bin"
export GUEST0_BUILD_SCRIPT="cd ../bmguest/ && \
make clean && \
make GUEST_NUMBER=0 && \
cp $BMGUEST_BIN ../../guestimages/ && \
cd ../../$GUEST0_DIR && \
make clean && \
make GUEST_NUMBER=0"
export GUEST0_CLEAN_SCRIPT="make clean"

export GUEST1_DIR="guestos/guestloader"
export GUEST1_BIN="guestloader.bin"
export GUEST1_BUILD_SCRIPT="cd ../bmguest/ && \
make clean && \
make GUEST_NUMBER=1 && \
cp $BMGUEST_BIN ../../guestimages/ && \
cd ../../$GUEST1_DIR && \
make clean && \
make GUEST_NUMBER=1"
export GUEST1_CLEAN_SCRIPT="make clean"

export GUEST2_DIR="guestos/guestloader"
export GUEST2_BIN="guestloader.bin"
export GUEST2_BUILD_SCRIPT="cd ../bmguest/ && \
make clean && \
make GUEST_NUMBER=2
cp $BMGUEST_BIN ../../guestimages/ && \
cd ../../$GUEST2_DIR && \
make clean && \
make GUEST_NUMBER=2"
export GUEST2_CLEAN_SCRIPT="make clean"

export GUEST3_DIR="guestos/guestloader"
export GUEST3_BIN="guestloader.bin"
export GUEST3_BUILD_SCRIPT="cd ../bmguest/ && \
make clean && \
make GUEST_NUMBER=3
cp $BMGUEST_BIN ../../guestimages/ && \
cd ../../$GUEST3_DIR && \
make clean && \
make GUEST_NUMBER=3"
export GUEST3_CLEAN_SCRIPT="make clean"

export GUEST_IMAGE_DIR="guestimages"
export CI_BUILD_DIR="bmguest_bmguest"
