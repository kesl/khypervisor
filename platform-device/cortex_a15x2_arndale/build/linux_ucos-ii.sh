#for bmguest + linux

export TARGET_PRODUCT="cortex_a15x2_arndale"

export GUEST_COUNT=2

export HYPERVISOR_BIN="hvc-man-switch.bin"
export HYPERVISOR_BUILD_SCRIPT="make clean && \
make LINUX=y RTOS=y CROSS_COMPILE=arm-linux-gnueabihf-"
export HYPERVISOR_CLEAN_SCRIPT="make clean"

export UBOOT_DIR="u-boot-native"
export UBOOT="u-boot.bin"
export UBOOT_BUILD_SCRIPT="make arndale5250 CROSS_COMPILE=arm-none-eabi-"
export UBOOT_CLEAN_SCRIPT="make clean"

export GUEST0_DIR="guestos/linux"
export GUEST0_BIN="arch/arm/boot/zImage"
export GUEST0_BUILD_SCRIPT="make ARCH=arm arndale_minimal_linux_defconfig && \
make CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm -j8"
export GUEST0_CLEAN_SCRIPT="make clean"

export GUEST1_DIR="guestos/ucos-ii"
export GUEST1_BIN="rtos.bin"
export GUEST1_BUILD_SCRIPT="make CROSS_COMPILE=arm-none-eabi-"
export GUEST1_CLEAN_SCRIPT="make clean"


export GUEST_IMAGE_DIR="guestimages"
export CI_BUILD_DIR="bmguest_linux"

