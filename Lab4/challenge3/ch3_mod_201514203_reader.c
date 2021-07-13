#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include "ch3.h"

int main(void) {
	int dev;

	struct msg_st user_msg1;
//	struct msg_st user_msg2;
//	struct msg_st user_msg3;
//	struct msg_st user_msg4;
//	struct msg_st user_msg5;

	dev = open("/dev/ch3_dev", O_RDWR);

	ioctl(dev, CH3_IOCTL_READ, &user_msg1);
	printf("%s\n", user_msg1.str);
//	ioctl(dev, CH3_IOCTL_READ, &user_msg2);
//	printf("%s\n", user_msg2.str);
//	ioctl(dev, CH3_IOCTL_READ, &user_msg3);
//	printf("%s\n", user_msg3.str);
//	ioctl(dev, CH3_IOCTL_READ, &user_msg4);
//	printf("%s\n", user_msg4.str);
//	ioctl(dev, CH3_IOCTL_READ, &user_msg5);

//	printf("%s\n", user_msg1.str);
//	printf("%s\n", user_msg2.str);
//	printf("%s\n", user_msg3.str);
//	printf("%s\n", user_msg4.str);
//	printf("%s\n", user_msg5.str);

	close(dev);
}
