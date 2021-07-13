#include <linux/init.h>
#include <linux/module.h>

static int my_id;
int get_my_id(void);
int set_my_id(int id);
EXPORT_SYMBOL(get_my_id);
EXPORT_SYMBOL(set_my_id);

static int __init ch1_mod1_init(void)
{
	return 0;
}

static void __exit ch1_mod1_exit(void)
{
}

int get_my_id(void)
{
	return my_id;
}

int set_my_id(int id)
{
	if(id)
	{
		my_id = id;
		return 1;
	}
	else
	{
		return 0;
	}
}

module_init(ch1_mod1_init);
module_exit(ch1_mod1_exit);
