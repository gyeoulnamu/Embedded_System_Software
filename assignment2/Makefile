obj-m:= ku.o

KERNELDIR:= ~/linux-rpi/
ARM:= ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi-
PWD:= $(shell pwd)

scp:
	scp $(obj-m:.o=_app) pi@10.10.10.12:~/
