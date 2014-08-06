#!/bin/bash
rm -rf ../linaro

set -e

EXACT=1
DIR=linaro

CPUS=`grep -c processor /proc/cpuinfo`

usage()
{
	echo 'Usage: $0 -c .config[ -t -d directory ]'
	echo -e " -c .config file   The kernel config file from the build. Download from:\n        http://snapshots.linaro.org/android/~linaro-android/arndale-linaro-14.06-release/1//kernel_config\n                   from a browser with cookies enabled."
	echo " -t                Reproduce the from the tip of the branch rather than doing"
	echo "                   an exact replica build"
	echo " -d <directory>    The directory to download code and build from"
	echo "                   Default: ${DIR}"
	exit 1
}

while getopts   "c:d:ht" optn; do
	case    $optn   in
		d   ) DIR=$OPTARG;;
		t   ) EXACT=0;;
		c   ) CFG=$OPTARG;;
		h   ) usage; exit 1;;
        esac
done

# download the kernel
if [ -d ${DIR} ] ; then
	echo "Directory ${DIR} exists. If the kernel code exists in this directory you may continue without cloning the git repository for the kernel. Are you sure you want to do this? (y/n) "
	read CONTINUE
	[ ${CONTINUE} == y ] || exit 1
else
	git clone git://git.linaro.org/landing-teams/working/samsung/kernel  $DIR
fi

cd $DIR
git checkout  5a93c058f6bd377fc5edad3e07b1f4d9f18f0c32

mv linaro ../


# download the kernel config
#curl -q http://snapshots.linaro.org/android/~linaro-android/arndale-linaro-14.06-release/1//kernel_config> linaro_kernel_config

# build the code
#CROSS_COMPILE=`which arm-linux-gnueabi-gcc |sed -e 's,gcc,,'`
#[ -d out ] || mkdir out
#[ -f out/.config ] || cp linaro_kernel_config out/.config
#if [[ `grep 'CONFIG_ARCH_MULTIPLATFORM=y' out/.config` ]]; then
#	KERNEL_IMAGE=zImage
#else
#	KERNEL_IMAGE=uImage
#fi
#make -j${CPUS} O=out ARCH=arm CROSS_COMPILE=$CROSS_COMPILE $KERNEL_IMAGE modules
#mkdir out/modules_for_android
#make O=out ARCH=arm modules_install INSTALL_MOD_PATH=modules_for_android
