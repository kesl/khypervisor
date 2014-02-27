# How to test guestloader

##Linux guest + bmguest<br>
Download linux using gitmodule command.
<pre>
cd {khypervisor_root_dir}
source platform-device/cortex_a15x2_arndale/build/linux_bmguest.sh
make
</pre>

##Linux guest + RTOS guest<br>
Download linux using gitmodule command.
<pre>
cd {khypervisor_root_dir}
source platform-device/cortex_a15x2_rtsm/build/linux_ucos-ii.sh
make
</pre>

##bmguest + bmguest
<pre>
cd {khypervisor_root_dir}
source platform-device/cortex_a15x2_rtsm/build/bmguest_bmguest.sh
make
</pre>

##How to Flash a K-hypervisor to arndale board
Please refer khypervisor/platform-device/cortex_a15x2_arndale/README.md
