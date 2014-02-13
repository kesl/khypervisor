# Hypervisor for ARMv7 Virtualization Extensions

## Basic directory structure
- hypervisor    Contains the real time hypervisor source code.
    - hvc call for manual guest switching
    - LPAE stage 2 address translation
    - Interrupt Handling through GICv2
    - Generic Timer and Scheduler (Round-robin)

- platform-device   Contains the device starting point.
    - common    common interface source code.
    - cortex_a15x2_arndale  based on arndale board code.
        - config    Contains the plaform specific configuration files.
        - drivers   Contains the plaform specific driver for hypervisor.
        - guestbl   Contains the guest bootloader.
        - guestimages   Contains the guest images.
        - guestos   Contains the guest os source code.
    - cortex_a15x2_rtsm  based on rtsm fastmodels code.
        - config    Contains the plaform specific configuration files.
        - drivers   Contains the plaform specific drivers for hypervisor.
        - guestbl   Contains the guest bootloader.
        - guestimages   Contains the guest images.
        - guestos   Contains the guest os source code.

The easiest way to use k-hypervisor is to start with one of the pre-configured 
platform-device projects (locate in the platform-device/<device_name> directory).  

See also -

    - arndale port : platform-device/cortex_a15x2_arndale
    - rtsm    port : platform-device/cortex_a15x2_rtsm

