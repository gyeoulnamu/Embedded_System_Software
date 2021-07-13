#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <asm/delay.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2

#define KU_IOCTL_NUM 'z'
#define KU_IOCTL_SENSE _IOWR(KU_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define KU_IOCTL_ACT _IOWR(KU_IOCTL_NUM, IOCTL_NUM2, unsigned long *)

MODULE_LICENSE("GPL");

#define PIN1 6
#define PIN2 13
#define PIN3 19
#define PIN4 26

#define DEV_NAME "ku_act_dev"
#define STEPS 8
#define ONEROUND 512

int blue[8] = {1, 1, 0, 0, 0, 0, 0, 1};
int pink[8] = {0, 1, 1, 1, 0, 0, 0, 0};
int yellow[8] = {0, 0, 0, 1, 1, 1, 0, 0};
int orange[8] = {0, 0, 0, 0, 0, 1, 1, 1};

spinlock_t my_lock;

void setstep(int p1, int p2, int p3, int p4) {
	gpio_set_value(PIN1, p1);
	gpio_set_value(PIN2, p2);
	gpio_set_value(PIN3, p3);
	gpio_set_value(PIN4, p4);
}

void forward(int round, int delay) {
	int i = 0, j = 0;

	for (i = 0; i < ONEROUND * round; i++) {
		for (j = 0; j <STEPS; j++) {
			setstep(blue[j], pink[j], yellow[j], orange[j]);
			udelay(delay);
		}
	}
	setstep(0, 0, 0, 0);
}

static int ku_act_receive(unsigned long arg) {
	int ret;
	int data;

	spin_lock(&my_lock);
	ret = copy_from_user(&data, (int*)arg, sizeof(int));
	spin_unlock(&my_lock);

	if (data != 0) {
		printk("Detect at %d secs\n", data);
		forward(1,3000);
	}

	return ret;
}

static long ku_act_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	int ret;

	switch (cmd) {
		case KU_IOCTL_ACT:
			ret = ku_act_receive(arg);
			break;
		default:
			return -1;
	}

	return 0;
}

static int ku_act_open(struct inode *inode, struct file *file) {
	printk("ku_act: open\n");
	return 0;
}

static int ku_act_release(struct inode *inode, struct file *file) {
	printk("ku_act: close\n");
	return 0;
}

struct file_operations ku_act_fops = {
	.unlocked_ioctl = ku_act_ioctl,
	.open = ku_act_open,
	.release = ku_act_release,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init ku_act_init(void) {
	printk("ku_act: Init Module\n");

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &ku_act_fops);
	cdev_add(cd_cdev, dev_num, 1);

	gpio_request_one(PIN1, GPIOF_OUT_INIT_LOW, "p1");
	gpio_request_one(PIN2, GPIOF_OUT_INIT_LOW, "p2");
	gpio_request_one(PIN3, GPIOF_OUT_INIT_LOW, "p3");
	gpio_request_one(PIN4, GPIOF_OUT_INIT_LOW, "p4");

	spin_lock_init(&my_lock);

	return 0;
}

static void __exit ku_act_exit(void) {
	printk("ku_act: Exit Module\n");
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);

	gpio_free(PIN1);
	gpio_free(PIN2);
	gpio_free(PIN3);
	gpio_free(PIN4);
}

module_init(ku_act_init);
module_exit(ku_act_exit);
