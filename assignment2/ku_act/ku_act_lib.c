#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2

#define KU_IOCTL_NUM 'z'
#define KU_IOCTL_SENSE _IOWR(KU_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define KU_IOCTL_ACT _IOWR(KU_IOCTL_NUM, IOCTL_NUM2, unsigned long *)

int ku_act(int data) {
	int dev, ret;

	dev = open("/dev/ku_act_dev", O_RDWR);

	ret = ioctl(dev, KU_IOCTL_ACT, &data);

	return ret;
}
