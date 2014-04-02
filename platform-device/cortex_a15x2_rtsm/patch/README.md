
* fs.cpio layout

<pre>
.:
benchmark  dev  etc  linuxrc  bin  demo  init  sbin  usr  proc  sys

./benchmark:
coremark  dhry2  dhry2reg  execl  context1  hanoi  pipe  syscall

./dev:
console

./etc:
init.d

./etc/init.d:
rcS

./bin:
base64  cttyhack  fsync     hush      login   mkdir   mpstat   pipe_progress  rpm    stat  true   cat     cpio   dumpkmap  fdflush  iostat    more        nice  rm            tar     dnsdomainname  run-parts  busybox
chmod   date      gunzip    ipcalc    ls      mknod   msh      printenv       sed    stty  uname  chattr  dd     echo      getopt   kill      mountpoint  ping  scriptreplay  umount  ln             touch
chown   egrep     gzip      kbd_mode  lsattr  mktemp  netstat  reformime      sh     su    vi     chgrp   df     ed        grep     linux64   mt          ps    setarch       usleep  ping6          watch
conspy  fgrep     hostname  linux32   lzop    mount   pidof    rmdir          sleep  sync  ash    cp      dmesg  false     ionice   makemime  mv          pwd   setserial     catv    rev            zcat

./demo:
syscall_test  test_instruction  test

./sbin:
acpid     blockdev    fbsplash     fsck        hdparm    ifenslave  ip       iprule    logread   man      mkfs.ext2   modinfo     poweroff     route       start-stop-daemon  switch_root  udhcpc
adjtimex  bootchartd  fdisk        fsck.minix  hwclock   ifup       ipaddr   iptunnel  losetup   mdev     mkfs.minix  modprobe    raidautorun  runlevel    sulogin            sysctl       vconfig
arp       depmod      findfs       getty       ifconfig  init       iplink   klogd     lsmod     mkdosfs  mkfs.vfat   nameif      reboot       setconsole  swapoff            syslogd      watchdog
blkid     devmem      freeramdisk  halt        ifdown    insmod     iproute  loadkmap  makedevs  mke2fs   mkswap      pivot_root  rmmod        slattach    swapon             tunctl       zcip

./usr:
bin  sbin

./usr/bin:
bzip2      pmap      unlzop    basename  comm       dos2unix    free     id       lpr       mkpasswd  pgrep     reset        setuidgid  telnet       tty       vlock    chpst   fold     passwd    split    uudecode  xz
crontab    runsv     unxz      beep      cryptpw    dumpleases  ftpget   ipcrm    lsof      nc        pkill     resize       sha256sum  test         ttysize   volname  cksum   ftpput   pscan     strings  wc
envuidgid  sha1sum   users     bunzip2   cut        env         fuser    ipcs     lzopcat   nmeter    printf    rpm2cpio     sha512sum  time         udpsvd    wall     cmp     hostid   realpath  sum      wget
killall    showkey   uuencode  cal       dc         expand      groups   last     md5sum    nohup     pstree    runsvdir     smemcap    timeout      unexpand  whoami   du      install  rx        sv       which
less       tftp      yes       chrt      deallocvt  fgconsole   hd       logger   mesg      nslookup  pwdx      script       softlimit  tr           uniq      xzcat    eject   lspci    seq       tac      who
lsusb      top       [         chvt      diff       find        head     logname  microcom  od        readlink  setkeycodes  tail       traceroute   unlzma    awk      envdir  lzcat    sha3sum   tee      whois
lzma       unix2dos  [[        clear     dirname    flock       hexdump  lpq      mkfifo    patch     renice    setsid       tcpsvd     traceroute6  uptime    bzcat    expr    openvt   sort      unzip    xargs

./usr/sbin:
addgroup   arping  chpasswd  delgroup   dnsd        fbset     httpd    killall5  nanddump    ntpd        rdate      readprofile   sendmail    svlogd   ubiattach  ubirmvol      udhcpd
add-shell  brctl   chroot    deluser    ether-wake  fdformat  ifplugd  loadfont  nandwrite   popmaildir  rdev       remove-shell  setfont     telnetd  ubidetach  ubirsvol
adduser    chat    crond     dhcprelay  fakeidentd  ftpd      inetd    lpd       nbd-client  powertop    readahead  rtcwake       setlogcons  tftpd    ubimkvol   ubiupdatevol

./proc:

./sys:

</pre>




