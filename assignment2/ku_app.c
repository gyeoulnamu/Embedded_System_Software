#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include "ku_sense/ku_sense_lib.c"
#include "ku_act/ku_act_lib.c"

int main(void) {
	int data;

	while(1) {
		ku_sense(&data);
		ku_act(data);
		sleep(1);
	}

	return 0;
}
