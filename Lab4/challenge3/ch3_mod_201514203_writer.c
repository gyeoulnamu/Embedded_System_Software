#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include "ch3.h"

int main(void) {
	int dev;
	struct msg_st user_msg1 = {
		0, "first msg!"
	};
/*	
	struct msg_st user_msg2 = {
		0, "second msg!"
	};
	
	struct msg_st user_msg3 = {
		0, "third msg!"
	};
	struct msg_st user_msg4 = {
		0, "fourth msg!"
	};
	struct msg_st user_msg5 = {
		0, "fifth msg!"
	};
*/	

	user_msg1.len = strlen(user_msg1.str);
//	user_msg2.len = strlen(user_msg2.str);
//	user_msg3.len = strlen(user_msg3.str);
//	user_msg4.len = strlen(user_msg4.str);
//	user_msg5.len = strlen(user_msg5.str);

	printf("Writer: Hello, reader!\n");

	dev = open("/dev/ch3_dev", O_RDWR);
	ioctl(dev, CH3_IOCTL_WRITE, &user_msg1);
//	ioctl(dev, CH3_IOCTL_WRITE, &user_msg2);
//	ioctl(dev, CH3_IOCTL_WRITE, &user_msg3);
//	ioctl(dev, CH3_IOCTL_WRITE, &user_msg4);
//	ioctl(dev, CH3_IOCTL_WRITE, &user_msg5);
	
	close(dev);
}
