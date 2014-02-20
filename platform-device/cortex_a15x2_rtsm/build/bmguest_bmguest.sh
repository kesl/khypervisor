#for bmguest + bmguest

export TARGET_PRODUCT="cortex_a15x2_rtsm"

export GUEST_COUNT=2

export HYPERVISOR_BIN="hvc-man-switch.axf"
export HYPERVISOR_BUILD_SCRIPT="make clean && \
make CROSS_COMPILE=arm-linux-gnueabihf-"
export HYPERVISOR_CLEAN_SCRIPT="make clean"

export UBOOT_DIR=""
export UBOOT=""
export UBOOT_BUILD_SCRIPT=""
export UBOOT_CLEAN_SCRIPT=""

export GUEST0_DIR="guestos/bmguest"
export GUEST0_BIN="bmguest.bin"
export GUEST0_BUILD_SCRIPT="make clean && \
make CROSS_COMPILE=arm-linux-gnueabihf-"
export GUEST0_CLEAN_SCRIPT="make clean"

export GUEST1_DIR="guestos/bmguest"
export GUEST1_BIN="bmguest.bin"
export GUEST1_BUILD_SCRIPT="make clean && \
make CROSS_COMPILE=arm-linux-gnueabihf-"
export GUEST1_CLEAN_SCRIPT="make clean"

export GUEST_IMAGE_DIR="guestimages"
export CI_BUILD_DIR="bmguest_bmguest"
