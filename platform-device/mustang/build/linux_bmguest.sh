#for bmguest + linux

export TARGET_PRODUCT="mustang"

export GUEST_COUNT=2

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
export GUEST0_BUILD_SCRIPT="cd ../../$GUEST0_DIR && \
make clean && \
make LINUX=y"
export GUEST0_CLEAN_SCRIPT="make clean"

export BM1_BIN="bmguest1.bin"
export GUEST1_DIR="guestos/guestloader"
export GUEST1_BIN="guestloader.bin"
export GUEST1_BUILD_SCRIPT="cd ../bmguest/ && \
make clean && \
make GUEST_NUMBER=1 && \
cp $BMGUEST_BIN ../../guestimages/ && \
cp $BMGUEST_BIN ../../guestimages/$BM1_BIN && \
cd ../../$GUEST1_DIR && \
make clean && \
make MONITOR=y"
export GUEST1_CLEAN_SCRIPT="make clean"

export GUEST_IMAGE_DIR="guestimages"
export CI_BUILD_DIR="bmguest_linux"
