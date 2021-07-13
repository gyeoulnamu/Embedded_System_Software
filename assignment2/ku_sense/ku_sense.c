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

#define SENSOR1 17

#define DEV_NAME "ku_sense_dev"

static int irq_num;
struct timespec start;
struct timespec recent;
static long time_stamp;

struct data_list {
	struct list_head list;
	int data;
};

static struct data_list data_list_head;
spinlock_t my_lock;

static void sensor_on(void) {
	enable_irq(irq_num);
	getnstimeofday(&start);
}

static void sensor_off(void) {
	disable_irq(irq_num);
}

static int ku_sense_send(unsigned long arg) {
	int ret;
	struct list_head *pos = NULL;
	struct data_list *tmp = NULL;

	list_for_each(pos, &(data_list_head.list)) {
		tmp = list_entry(pos, struct data_list, list);
		if (tmp->data == NULL) {
			printk("ku_sense: no data sensed\n");
		} else {
			spin_lock(&my_lock);
			ret = copy_to_user((int*)arg, &tmp->data, sizeof(unsigned long));
			spin_unlock(&my_lock);
		}
	}

	return ret;
}

static long ku_sense_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	int ret;
	
	switch (cmd) {
		case KU_IOCTL_SENSE:
			ret = ku_sense_send(arg);
			break;
		default:
			return -1;
	}

	return 0;
}

static int ku_sense_open(struct inode *inode, struct file *file) {
	printk("ku_sense: open\n");
	sensor_on();

	return 0;
}

static int ku_sense_release(struct inode *inode, struct file *file) {
	printk("ku_sense: close\n");
	sensor_off();

	return 0;
}

struct file_operations ku_sense_fops = {
	.unlocked_ioctl = ku_sense_ioctl,
	.open = ku_sense_open,
	.release = ku_sense_release,
};

static irqreturn_t ku_sense_isr(int irq, void* dev_id) {
	unsigned long flags;
	struct data_list *tmp = 0;

	local_irq_save(flags);

	getnstimeofday(&recent);
	time_stamp = recent.tv_sec - start.tv_sec;
	tmp = (struct data_list*)kmalloc(sizeof(struct data_list), GFP_KERNEL);
	tmp->data = (int)time_stamp;
	list_add(&tmp->list, &data_list_head.list);

	local_irq_restore(flags);
	return IRQ_HANDLED;
}

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init ku_sense_init(void) {
	int ret;
	struct data_list *tmp = 0;

	printk("ku_sense: Init Module\n");

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &ku_sense_fops);
	cdev_add(cd_cdev, dev_num, 1);

	gpio_request_one(SENSOR1, GPIOF_IN, "sensor1");
	irq_num = gpio_to_irq(SENSOR1);
	ret = request_irq(irq_num, ku_sense_isr, IRQF_TRIGGER_RISING, "sensor_irq", NULL);
	if(ret) {
		printk("ku_sense: Unable to request IRQ: %d\n", irq_num);
		free_irq(irq_num, NULL);
	} else {
		disable_irq(irq_num);
	}

	INIT_LIST_HEAD(&data_list_head.list);
	spin_lock_init(&my_lock);

	tmp = (struct data_list*)kmalloc(sizeof(struct data_list), GFP_KERNEL);
	tmp->data = NULL;
	list_add(&tmp->list, &data_list_head.list);

	return 0;
}

static void __exit ku_sense_exit(void) {
	printk("ku_sense: Exit Module\n");
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);

	free_irq(irq_num, NULL);
	gpio_free(SENSOR1);
}

module_init(ku_sense_init);
module_exit(ku_sense_exit);
