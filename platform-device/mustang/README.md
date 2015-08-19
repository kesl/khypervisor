# Hypervisor for ARMv8 Virtualization Extensions

## Initializing a Build Environment

### Download Tool chain for k-hypervisor
- Download <i>aarch64-apm-linux-gnu</i> or <i>aarch64-linux-gnu</i>
- You can download it from linaro here is link
<a href ="https://releases.linaro.org/13.09/components/toolchain/binaries/gcc-linaro-aarch64-linux-gnu-4.8-2013.09_linux.tar.xz">aarch64-linux-gnu</a>

### Install tftp
- Install tftp and create configuration file
<pre>
sudo apt-get install xinetd tftp tftpd
sudo vi /etc/xinetd.d/tftp
</pre>
<pre>
service tftp
{
    socket_type     = dgram
    protocol        = udp
    wait            = yes
    user            = root
    server          = /usr/sbin/in.tftpd
    server_args     = -s /tftpboot
    disable         = no
    per_source      = 11
    cps             = 100 2
    flags           = IPv4
}
</pre>

- make tftpboot directory
<pre>
sudo mkdir /tftpboot
sudo chmod 777 /tftpboot
mkdir /tftpboot/khyp
</pre>

- Restart server
<pre>
sudo /etc/init.d/xinetd restart
</pre>

### Setting up khypervisor booting environment
- Setting u-boot environment in mustang board
<pre>
setenv ipaddr # IP address of board
setenv serverip # IP address of server
setenv ethaddr # MAC of Ethernet (server)
setenv user_dir khyp
setenv gatewayip 10.x.x.x
setenv netmask 255.x.x.x
saveenv

setenv loader0_addr_r 0x4000000000
setenv loader1_addr_r 0x4080000000
setenv bm0_addr_r 0x4000500000
setenv bm1_addr_r 0x4080500000
setenv khyp_addr_r 0x41e0000000

setenv khyp_load 'tftp ${khyp_addr_r} ${user_dir}/hvm.bin'
setenv loader0_load 'tftp ${loader0_addr_r} ${user_dir}/guest0.bin'
setenv loader1_load 'tftp ${loader1_addr_r} ${user_dir}/guest1.bin'
setenv bm0_load 'tftp ${bm0_addr_r} ${user_dir}/bmguest0.bin'
setenv bm1_load 'tftp ${bm1_addr_r} ${user_dir}/bmguest1.bin'
setenv khyp_run 'run khyp_load loader0_load bm0_load loader1_load bm1_load;go ${khyp_addr_r}'
saveenv
</pre>

# Download hypervisor
<pre>
$ git clone https://github.com/kesl/khypervisor.git -b v1-dev-mustang
$ cd khypervisor
$ git submodule init
$ git submodule update
$ ./scripts/apply_patch.sh
</pre>

# How to test bmguest + bmguest

## Make a build in one step continuous integration
Go to "Testing bmguest on mustang"
this section, if you done this process first.
<pre>
$ cd khypervisor
$ source platform-device/mustang/build/bmguest_bmguest.sh
$ make
</pre>

## Building the bmguest
<pre>
$ cd platform-device/mustang/guestos/bmguest/
$ make
</pre>

## Building the guest loader
1. Copy guest image to guestimages directory
<pre>
$ cd khypervisor/platform-device/mustang
$ cp ./guestos/bmguest/bmguest.bin ./guestimages/bmguest.bin
</pre>
2. Build guestloader for bmguest
<pre>
$ cd khypervisor/platform-device/mustang/guestos/guestloader
$ make
$ cp guestloader.bin ../../guestimages/guest0.bin
$ cp guestloader.bin ../../guestimages/guest1.bin
</pre>

## Building the k-hypervisor
<pre>
$ cd khypervisor/platform-device/mustang
$ make
</pre>

## Testing bmguest on mustang
1. To use tftp, move guest, guestloader and hypervisor to tftpboot directory.
<pre>
$ cd khypervisor/platform-device/mustang
$ tftp=/tftpboot/khyp/
$ sudo cp ./hvc-man-switch.bin ${tftp}/hvm.bin
$ sudo cp ./guestimages/bmguest0.bin ${tftp}/
$ sudo cp ./guestimages/bmguest1.bin ${tftp}/
$ sudo cp ./guestimages/guest0.bin ${tftp}/
$ sudo cp ./guestimages/guest1.bin ${tftp}/
</pre>
2. Run khypervisor in mustang board. It start at u-boot
<pre>
(u-boot) $ khyp_run
</pre>
# How to test bmguset + linux guest

## Make a build in one step continuous integration
Go to "Testing bmguest+linuxguest on mustang"
this section, if you done this process first.
<pre>
$ cd khypervisor
$ source platform-device/mustang/build/linux_bmguest.sh
$ make
</pre>

## Building the bmguest
<pre>
$ cd platform-device/mustang/guestos/bmguest/
$ make
</pre>

## Building the linux guset
1. Building linux guest
<pre>
</pre>

## Build guest loader
1. Copy guest image to guestimages directory
<pre>
$ cd khypervisor/platform-device/mustang
$ cp ./guestos/linux/arch/arm/boot/zImage ./guestimages/zImage
$ cp ./guestos/bmguest/bmguest.bin ./guestimages/bmguest.bin
</pre>
2. Build guestloader for linux guest
<pre>
$
$ cd khypervisor/platform-device/mustang/guestos/guestloader
$ make LINUX=y
$ cp guestloader.bin ../../guestimages/guest0.bin
</pre>
3. Build guestloader for bmguest
<pre>
$ cd khypervisor/platform-device/mustang/guestos/guestloader
$ make
$ cp guestloader.bin ../../guestimages/guest1.bin
</pre>

## Building the k-hypervisor
<pre>
$ cd platform-device/mustang
$ make
</pre>

## Testing bmguest + linux guest on mustang
1.
