#for bmguest + linux

export TARGET_PRODUCT="cortex_a15x2_rtsm"

export GUEST_COUNT=2

export HYPERVISOR_BIN="hvc-man-switch.axf"
export HYPERVISOR_BUILD_SCRIPT="make clean && \
make CROSS_COMPILE=arm-linux-gnueabihf- LINUX=y"
export HYPERVISOR_CLEAN_SCRIPT="make clean"

export UBOOT_DIR=""
export UBOOT=""
export UBOOT_BUILD_SCRIPT=""
export UBOOT_CLEAN_SCRIPT=""

export GUEST0_DIR="guestos/linux"
export GUEST0_BIN="arch/arm/boot/zImage"
export GUEST0_BUILD_SCRIPT="make ARCH=arm vexpress_minhw_defconfig && \
cp -a ../../linuxguest/fs.cpio . && \
cp -a ../../linuxguest/host-a15.dtb . && \
make CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm -j8 && \
cat host-a15.dtb >> arch/arm/boot/zImage"
export GUEST0_CLEAN_SCRIPT="make clean"

export GUEST1_DIR="guestos/bmguest"
export GUEST1_BIN="bmguest.bin"
export GUEST1_BUILD_SCRIPT="make clean && \
make CROSS_COMPILE=arm-linux-gnueabihf-"
export GUEST1_CLEAN_SCRIPT="make clean"

export GUEST_IMAGE_DIR="guestimages"
export CI_BUILD_DIR="bmguest_linux"

