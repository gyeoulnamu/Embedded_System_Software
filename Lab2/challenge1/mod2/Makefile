obj-m := ch1_mod2_201514203.o

KBUILD_EXTRA_SYMBOLS := /home/gyeoulnamu/Class/Lab2/challenge1/mod1/Module.symvers
KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

