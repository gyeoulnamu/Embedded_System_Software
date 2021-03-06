#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

#define SWITCH 12

static int count = 0;

static int __init simple_switch_init(void) {
	int ret = 0;
	printk("simple_switch: init module\n");
	gpio_request_one(SWITCH, GPIOF_IN, "SWITCH");

	while(count != 5) {
		ret = gpio_get_value(SWITCH);
		printk("simple_switch: ret = %d\n", ret);
		if(ret) {
			count++;
			printk("simple_switch: pushed button, count = %d\n", count);
		}
		msleep(1000);
	}
	printk("simple_switch: finish pushed button\n");

	return 0;
}

static void __exit simple_switch_exit(void) {
	printk("simple_switch: exit module\n");

	gpio_free(SWITCH);
}

module_init(simple_switch_init);
module_exit(simple_switch_exit);
