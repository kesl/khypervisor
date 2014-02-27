# How to test guestloader

##Linux guest + bmguest<br>
Download linux using gitmodule command.
<pre>
cd {khypervisor_root_dir}
source platform-device/cortex_a15x2_rtsm/build/linux_bmguest.sh
make
cd platform-device/cortex_a15x2_rtsm
RTSM_VE_Cortex-A15x1-A7x1 -a coretile.cluster0.cpu0=hvc-man-switch.axf
</pre>

##bmguest + bmguest
<pre>
cd {khypervisor_root_dir}
source platform-device/cortex_a15x2_rtsm/build/bmguest_bmguest.sh
make
cd platform-device/cortex_a15x2_rtsm
RTSM_VE_Cortex-A15x1-A7x1 -a coretile.cluster0.cpu0=hvc-man-switch.axf
</pre>
