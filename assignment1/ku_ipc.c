#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <asm/delay.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/string.h>
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

#define DEV_NAME "ku_ipc_dev"
MODULE_LICENSE("GPL");
spinlock_t my_lock;
wait_queue_head_t my_wq;
static long my_data;
static dev_t dev_num;
static struct cdev* cd_cdev;

struct msgbuf {
	long type;
	char text[128];
};

struct msg_args {
	int msqid, msgsz, msgflg;
	void *msgp;
	long msgtyp;
};

struct msg_list {
	struct list_head list;
	struct msgbuf msg;
	int reference_counter;
	int msg_counter;
	int msg_volume;
};

static struct msg_list msg_list_head[10];

void delay(int sec) {
	int i, j;
	for (j = 0; j < sec; j++) {
		for (i = 0; i < 1000; i++) {
			udelay(1000);
		}
	}
}

static int ku_msgget(int key, int msgflg) {
	printk("ku_ipc: flag = %d\n", msgflg);
	
	if (msgflg == KU_IPC_CREAT) {
		msg_list_head[key].reference_counter++;
		printk("ku_ipc: (get) key = %d, value = %d\n", key, msg_list_head[key].reference_counter);
		
		return key;
	}
	else if (msgflg == KU_IPC_EXCL) {
		if(msg_list_head[key].reference_counter == 0) {
			msg_list_head[key].reference_counter++;
			printk("ku_ipc: (get) key = %d, value = %d\n", key, msg_list_head[key].reference_counter);
			return key;
		}
		else {
			printk("ku_ipc: queue is using\n");
			return -1;
		}
	}
	else {
		return -1;
	}
}

static int ku_msgclose(int msqid) {
	if (msqid < 0 || msqid >= 10) {
		return -1;
	}
	else if (msg_list_head[msqid].reference_counter > 0) {
		msg_list_head[msqid].reference_counter--;
		printk("ku_ipc: (close) key = %d, value = %d\n", msqid, msg_list_head[msqid].reference_counter);
		return 0;
	}
	else {
		printk("ku_ipc: queue is not using\n");
		return -1;
	}
}

static int ku_msgsnd(int msqid, void* msgp, int msgsz, int msgflg) {
	int ret;
	struct msg_list* tmp = NULL;

	if (msg_list_head[msqid].msg_counter >= KUIPC_MAXMSG || msg_list_head[msqid].msg_volume + msgsz > KUIPC_MAXVOL) {
		if (msgflg == KU_IPC_NOWAIT) {
			printk("ku_ipc: no space (flag = %d)\n", msgflg);
			ret = -1;
		}
		else {
			printk("ku_ipc: no space, please wait (flag = %d)\n", msgflg);
			ret = wait_event_interruptible(my_wq, my_data > 0);
			ret = ku_msgsnd(msqid, msgp, msgsz, msgflg);
			if(ret < 0) {
				return ret;
			}
			ret = 0;
		}
	}
	else {
		tmp = (struct msg_list*)kmalloc(sizeof(struct msg_list), GFP_KERNEL);

		spin_lock(&my_lock);
		ret = copy_from_user(&tmp->msg, (struct msgbuf*)msgp, sizeof(struct msgbuf));
		spin_unlock(&my_lock);

		printk("ku_ipc: key = %d, content = %c\n", msqid, tmp->msg.text[0]);
		list_add_tail(&tmp->list, &msg_list_head[msqid].list);
		msg_list_head[msqid].msg_counter++;
		msg_list_head[msqid].msg_volume += msgsz;
		my_data++;

		wake_up_interruptible(&my_wq);
		ret = 0;
	}

	return ret;
}

static int ku_msgrcv(int msqid, void* msgp, int msgsz, long msgtyp, int msgflg) {
	int ret;
	struct msg_list* tmp = NULL;
	struct list_head* pos = NULL;
	struct list_head* q = NULL;
	
	ret = -1;
	pos = &msg_list_head[msqid].list;
	if(list_empty(pos)) {
		if (msgflg == KU_IPC_NOWAIT) {
			printk("ku_ipc: no data (flag = %d)\n", msgflg);
			ret = -1;
		}
		else {
			printk("ku_ipc: no data, please wait (flag = %d)\n", msgflg);
			ret = wait_event_interruptible(my_wq, my_data > 0);
			ret = ku_msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
		}
	}
	else {
		list_for_each_safe(pos, q, &msg_list_head[msqid].list) {
			tmp = list_entry(pos, struct msg_list, list);
			if (tmp->msg.type == msgtyp) {
				if(strlen(tmp->msg.text) > msgsz) {
					if(msgflg == KU_MSG_NOERROR) {	
						tmp->msg.text[msgsz-1] = '\0';
						
						delay(1);
						spin_lock(&my_lock);
						ret = copy_to_user((struct msgbuf*)msgp, &tmp->msg, sizeof(struct msgbuf));
						spin_unlock(&my_lock);

						printk("ku_ipc: data is deleted (flag = %d)\n", msgflg);
						list_del(pos);
						kfree(tmp);
						msg_list_head[msqid].msg_counter--;
						msg_list_head[msqid].msg_volume -= strlen(tmp->msg.text);
						my_data++;
						ret = msgsz;
						wake_up_interruptible(&my_wq);
						break;
					}
					else {
						printk("ku_ipc: data is too big (flag = %d)\n", msgflg);
						ret = -1;
						break;
					}
				}
				else {
					delay(1);
					spin_lock(&my_lock);
					ret = copy_to_user((struct msgbuf*)msgp, &tmp->msg, sizeof(struct msgbuf));
					spin_unlock(&my_lock);

					printk("ku_ipc: data is deleted\n");
					list_del(pos);
					kfree(tmp);
					msg_list_head[msqid].msg_counter--;
					msg_list_head[msqid].msg_volume -= strlen(tmp->msg.text);
					
					my_data++;
					ret = strlen(tmp->msg.text);
					wake_up_interruptible(&my_wq);
					break;
				}
			}
			ret = -1;
			break;
		}
		if (ret < 0) {
			return ret;
		}
	}

	return ret;
}

static long ku_ipc_ioctl(struct file* file, unsigned int cmd, unsigned long arg) {
	int ret;
	struct msg_args* user_arg;
	user_arg = (struct msg_args*)arg;
	
	switch(cmd) {
		case KU_MSGGET:
			ret = ku_msgget(user_arg->msqid, user_arg->msgflg);
			break;
		case KU_MSGCLOSE:
			ret = ku_msgclose(user_arg->msqid);
			break;
		case KU_MSGSND:
			ret = ku_msgsnd(user_arg->msqid, user_arg->msgp, user_arg->msgsz, user_arg->msgflg);
			break;
		case KU_MSGRCV:
			ret = ku_msgrcv(user_arg->msqid, user_arg->msgp, user_arg->msgsz, user_arg->msgtyp, user_arg->msgflg);
			break;
	}

	return 0;
}

static int ku_ipc_open(struct inode *inode, struct file *file) {
	printk("ku_ipc: open\n");
	return 0;
}

static int ku_ipc_release(struct inode *inode, struct file *file) {
	printk("ku_ipc: release\n");
	return 0;
}

struct file_operations ku_ipc_fops = {
	.unlocked_ioctl = ku_ipc_ioctl,
	.open = ku_ipc_open,
	.release = ku_ipc_release,
};

static int __init ku_ipc_init(void) {
	int ret;
	int i;

	printk("ku_ipc: init module\n");

	for (i = 0; i < 10; i++) {
		INIT_LIST_HEAD(&msg_list_head[i].list);
		msg_list_head[i].reference_counter = 0;
		msg_list_head[i].msg_counter = 0;
		msg_list_head[i].msg_volume = 0;
	}
	spin_lock_init(&my_lock);
	init_waitqueue_head(&my_wq);

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &ku_ipc_fops);
	ret = cdev_add(cd_cdev, dev_num, 1);

	if (ret < 0) {
		printk("ku_ipc: fail to add character device\n");
		return -1;
	}

	return 0;
}

static void __exit ku_ipc_exit(void) {
	printk("ku_ipc: exit module\n");

	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(ku_ipc_init);
module_exit(ku_ipc_exit);
