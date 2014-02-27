#for bmguest + linux

export TARGET_PRODUCT="cortex_a15x2_rtsm"

export GUEST_COUNT=2

export HYPERVISOR_BIN="hvc-man-switch.axf"
export HYPERVISOR_BUILD_SCRIPT="make clean && \
make"
export HYPERVISOR_CLEAN_SCRIPT="make clean"

export UBOOT_DIR=""
export UBOOT=""
export UBOOT_BUILD_SCRIPT=""
export UBOOT_CLEAN_SCRIPT=""

export ZIMAGE_BIN="zImage"
export BMGUEST_BIN="bmguest.bin"
export GUEST0_DIR="guestos/guestloader"
export GUEST0_BIN="guestloader.bin"
export GUEST0_BUILD_SCRIPT="cd ../linux && \
make ARCH=arm vexpress_minhw_defconfig && \
cp -a ../../linuxguest/fs.cpio . && \
cp -a ../../linuxguest/host-a15.dtb . && \
make CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm -j8 && \
cat host-a15.dtb >> arch/arm/boot/zImage && \
cp arch/arm/boot/zImage ../../guestimages/ && \
cd ../../$GUEST0_DIR && \
make clean && \
make LINUX=y"
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
export CI_BUILD_DIR="bmguest_linux"
