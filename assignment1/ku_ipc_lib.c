#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include "ku_ipc.h"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2
#define IOCTL_NUM3 IOCTL_START_NUM+3
#define IOCTL_NUM4 IOCTL_START_NUM+4

#define KU_IOCTL_NUM 'z'
#define KU_MSGGET _IOWR(KU_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define KU_MSGCLOSE _IOWR(KU_IOCTL_NUM, IOCTL_NUM2, unsigned long *)
#define KU_MSGSND _IOWR(KU_IOCTL_NUM, IOCTL_NUM3, unsigned long *)
#define KU_MSGRCV _IOWR(KU_IOCTL_NUM, IOCTL_NUM4, unsigned long *)

struct msgbuf {
	long type;
	char text[128];
};

struct msg_args {
	int msqid, msgsz, msgflg;
	void* msgp;
	long msgtyp;
};

struct msg_args user_arg;

int ku_msgget(int key, int msgflg) {
	int dev, ret;
	
	user_arg.msqid = key;
	user_arg.msgflg = msgflg;

	dev = open("/dev/ku_ipc_dev", O_RDWR);
	ret = ioctl(dev, KU_MSGGET, &user_arg);
	printf("success to get?: %d\n", ret);

	close(dev);
	
	return ret;
}

int ku_msgclose(int msqid) {
	int dev, ret;
	
	user_arg.msqid = msqid;

	dev = open("/dev/ku_ipc_dev", O_RDWR);
	ret = ioctl(dev, KU_MSGCLOSE, &user_arg);
	printf("success to close?: %d\n", ret);

	close(dev);

	return ret;
}

int ku_msgsnd(int msqid, void *msgp, int msgsz, int msgflg) {
	int dev, ret;

	user_arg.msqid = msqid;
	user_arg.msgp = msgp;
	user_arg.msgsz = msgsz;
	user_arg.msgflg = msgflg;

	dev = open("/dev/ku_ipc_dev", O_RDWR);
	ret = ioctl(dev, KU_MSGSND, &user_arg);
	printf("success to send?: %d\n", ret);

	close(dev);

	return ret;
}

int ku_msgrcv(int msqid, void *msgp, int msgsz, long msgtyp, int msgflg) {
	int dev, ret;

	user_arg.msqid =  msqid;
	user_arg.msgp = msgp;
	user_arg.msgsz = msgsz;
	user_arg.msgtyp = msgtyp;
	user_arg.msgflg = msgflg;

	dev = open("/dev/ku_ipc_dev", O_RDWR);
	ret = ioctl(dev, KU_MSGRCV, &user_arg);
	printf("success to receive?: %d\n", ret);

	close(dev);

	return ret;
}
