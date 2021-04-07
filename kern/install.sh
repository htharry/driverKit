#umount /vafs
rmmod va
insmod va.ko
#mount -t vafs vafs /vafs
