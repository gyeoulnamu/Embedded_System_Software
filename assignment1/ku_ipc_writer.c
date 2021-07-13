#include "ku_ipc_lib.c"

void main() {
	int ret;
	int key = 0;
	int msqid;
	int msgsize;
	struct msgbuf user_msg = {1, "Empty"};

	printf("input the message: ");
	fgets(user_msg.text, 128, stdin);
	//user_msg.text[strlen(user_msg.text) - 1] = '\0';

	printf("input type of the message: ");
	scanf("%ld", &user_msg.type);

	msgsize = strlen(user_msg.text);
	msqid = ku_msgget(key, KU_IPC_CREAT);
	printf("key: %d\n", key);

	ret = ku_msgsnd(msqid, &user_msg, msgsize, KU_IPC_NOWAIT);
}
