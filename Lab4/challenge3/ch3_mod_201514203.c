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

#include "ch3.h"
#define DEV_NAME "ch3_dev"

MODULE_LICENSE("GPL");
struct msg_st *kern_buf;
spinlock_t my_lock;

struct msg_list {
	struct list_head list;
	struct msg_st msg;
};

static struct msg_list msg_list_head;

void delay(int sec) {
	int i, j;
	for (j = 0; j < sec; j++) {
		for(i = 0; i < 1000; i++) {
			udelay(1000);
		}
	}
}

static void ch3_ioctl_read(struct msg_st *buf) {
	struct msg_list* tmp = 0;
	struct list_head* pos = 0;
	struct list_head* q = 0;
	struct msg_st msg_null = {
		0, '\0'
	};

	delay(1);
	spin_lock(&my_lock);
	list_for_each_safe(pos, q, &msg_list_head.list) {
		if(!list_empty(&msg_list_head.list)) {
			tmp = list_entry(pos, struct msg_list, list);
//			memcpy(kern_buf, &tmp->msg, sizeof(struct msg_list));
//			if(list_empty(&msg_list_head.list)) {
//				memcpy(kern_buf, &(msg_null), sizeof(struct msg_st));
//			}
//			copy_to_user(buf, kern_buf, sizeof(struct msg_st));
			copy_to_user(buf, &tmp->msg, sizeof(struct msg_st));
//			memset(kern_buf, '\0', sizeof(struct msg_st));
			printk("ch3_mod_201514203: message is deleted");
			list_del(pos);
			kfree(tmp);
			break;
		}
		else {
			printk("ch3_mod_201514203: Empty List");
			copy_to_user(buf, &msg_null, sizeof(struct msg_st));
		}	
	}	
	spin_unlock(&my_lock);
}

static void ch3_ioctl_write(struct msg_st *buf) {
	struct msg_list* tmp = 0;
	tmp = (struct msg_list*)kmalloc(sizeof(struct msg_list), GFP_KERNEL);

	spin_lock(&my_lock);
//	copy_from_user(kern_buf, buf, sizeof(struct msg_st));
//	tmp = (struct msg_list*)kmalloc(sizeof(struct msg_list), GFP_KERNEL);
	copy_from_user(&tmp->msg, buf, sizeof(struct msg_st));
//	memcpy(&tmp->msg, kern_buf, sizeof(struct msg_st)); //tmp->msg = *kern_buf;
	printk("ch3_mod_201514203: message is stored");
//	list_add_tail(&tmp->list, &msg_list_head.list);	
	spin_unlock(&my_lock);

	list_add_tail(&tmp->list, &msg_list_head.list);
}

static long ch3_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	struct msg_st* user_buf;
	user_buf = (struct msg_st*)arg;

	switch(cmd) {
		case CH3_IOCTL_READ:
			ch3_ioctl_read(user_buf);
			printk("ch3_mod_201514203: ioctl read complete");
			break;
		case CH3_IOCTL_WRITE:
			ch3_ioctl_write(user_buf);
			printk("ch3_mod_201514203: ioctl write complete");
			break;
	}

	return 0;
}

static int ch3_ioctl_open(struct inode *inode, struct file *file) {
	printk("ch3_mod_201514203: open\n");
	return 0;
}

static int ch3_ioctl_release(struct inode *inode, struct file *file) {
	printk("ch3_mod_201514203: release\n");
	return 0;
}

struct file_operations ch3_ioctl_fops = {
	.unlocked_ioctl = ch3_ioctl,
	.open = ch3_ioctl_open,
	.release = ch3_ioctl_release
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init ch3_ioctl_init(void) {
	int ret;

	printk("ch3_mod_201514203: Init Module\n");
	INIT_LIST_HEAD(&msg_list_head.list);
	spin_lock_init(&my_lock);
	
	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &ch3_ioctl_fops);
	ret = cdev_add(cd_cdev, dev_num, 1);
	if (ret < 0) {
		printk("fail to add character device \n");
		return -1;
	}

	kern_buf = (struct msg_st*)vmalloc(sizeof(struct msg_st));
	memset(kern_buf, '\0', sizeof(struct msg_st));

	return 0;
}

static void __exit ch3_ioctl_exit(void) {
	printk("ch3_mod_201514203: Exit Module\n");
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
	vfree(kern_buf);
}

module_init(ch3_ioctl_init);
module_exit(ch3_ioctl_exit);
