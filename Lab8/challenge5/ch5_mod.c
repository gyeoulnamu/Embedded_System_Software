#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

#define LED1 5
#define SWITCH 12

struct my_timer_info {
	struct timer_list timer;
	long delay_jiffies;
	int data;
};

static struct my_timer_info my_timer;

static void my_timer_func(struct timer_list *t) {
	struct my_timer_info *info = from_timer(info, t, timer);

	info->data = gpio_get_value(SWITCH);
	gpio_set_value(LED1, info->data);
	printk("ch5_mod: jiffies: %ld, switch: %d\n", jiffies, info->data);

	mod_timer(&my_timer.timer, jiffies + info->delay_jiffies);
}

static int __init ch5_mod_init(void) {
	printk("ch5_mod: init module\n");

	gpio_request_one(LED1, GPIOF_OUT_INIT_LOW, "LED1");
	gpio_request_one(SWITCH, GPIOF_IN, "SWITCH");

	my_timer.delay_jiffies = msecs_to_jiffies(5000);
	my_timer.data = 0;
	timer_setup(&my_timer.timer, my_timer_func, 0);
	my_timer.timer.expires = jiffies + my_timer.delay_jiffies;
	add_timer(&my_timer.timer);

	return 0;
}

static void __exit ch5_mod_exit(void) {
	printk("ch5_mod: exit module\n");

	gpio_free(LED1);
	gpio_free(SWITCH);
	del_timer(&my_timer.timer);
}

module_init(ch5_mod_init);
module_exit(ch5_mod_exit);
