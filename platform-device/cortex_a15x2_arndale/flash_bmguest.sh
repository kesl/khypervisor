sudo dd if=hvc-man-switch.bin of=/dev/sdb bs=512 seek=1105
sudo dd if=guestimages/guest0.bin of=/dev/sdb bs=512 seek=1205
sudo dd if=guestimages/guest1.bin of=/dev/sdb bs=512 seek=17589
sudo dd if=guestimages/bmguest.bin of=/dev/sdb bs=512 seek=17789
sudo dd if=guestimages/bmguest.bin of=/dev/sdb bs=512 seek=34173
sudo dd if=guestimages/System.map of=/dev/sdb bs=512 seek=34373
sudo sync
sudo umount /dev/sdb*
