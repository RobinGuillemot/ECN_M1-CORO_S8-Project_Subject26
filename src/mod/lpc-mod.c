#include <linux/configfs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/umh.h>
#include <linux/workqueue.h>

MODULE_LICENSE("GPL");

#define APP_PATH "/usr/lib/ecn/lpc"

static unsigned int batIntrptPeriod = 1000;

struct changeParamsFs
{
	struct configfs_subsystem periods;
};

static inline struct changeParamsFs *
	to_changeParamsFs (struct config_item *item)
{
	return item ? container_of(to_configfs_subsystem(to_config_group(item)),
		struct changeParamsFs, periods) : NULL;
}

static ssize_t changeParamsFs_batIntrptPeriod_show (struct config_item *item,
						char* page)
{
	return sprintf(page, "%d\n", batIntrptPeriod);
}

static ssize_t changeParamsFs_batIntrptPeriod_store(struct config_item *item,
							 const char *page,
							 size_t count)
{
	unsigned long tmp;
	char *p = (char *)page;

	tmp = simple_strtoul(p, &p, 10);
	if (!p || (*p && (*p != '\n')))
		return -EINVAL;

	if (tmp > UINT_MAX)
		return -ERANGE;

	batIntrptPeriod = tmp;

	return count;
}

static ssize_t changeParamsFs_description_show
	(struct config_item *item,
	 char *page)
{
	return sprintf (page,
"   Allows to modify the interrupts periods in the\n"
"lpc-mod kernel module. Modifiable attributes are:\n"
"\t- batIntrptPeriod.\n");
}

CONFIGFS_ATTR(changeParamsFs_, batIntrptPeriod);
CONFIGFS_ATTR_RO(changeParamsFs_, description);

static struct configfs_attribute *changeParamsFs_attrs[] =
{
	&changeParamsFs_attr_batIntrptPeriod,
	&changeParamsFs_attr_description,
	NULL
};

static const struct config_item_type changeParamsFs_type =
{
	.ct_attrs = changeParamsFs_attrs,
	.ct_owner = THIS_MODULE
};

static struct changeParamsFs changeParamsFs_periods =
{
	.periods =
	{
		.su_group =
		{
			.cg_item =
			{
				.ci_namebuf = "ecn_lpc_mod",
				.ci_type = &changeParamsFs_type
			},
		},
	}
};

static struct configfs_subsystem *subsys = &changeParamsFs_periods.periods;

static int changeParamsFs_init (void)
{
	int err;

	changeParamsFs_attr_batIntrptPeriod.ca_mode = 0666;
	config_group_init(&subsys->su_group);
	mutex_init(&subsys->su_mutex);
	err = configfs_register_subsystem(subsys);
	if (err)
	{
		printk(KERN_ERR "Error %d while registering subsystem %s\n",
				err,
				subsys->su_group.cg_item.ci_namebuf);
		goto out_unregister;
	}

	return 0;

out_unregister:

	configfs_unregister_subsystem(subsys);

	return err;

}

static void changeParamsFs_exit (void)
{
	configfs_unregister_subsystem(subsys);
}

static char *battery_args[] = {APP_PATH, NULL, NULL, NULL};
static char *envp[] = {
			"HOME=/",
			"TERM=linux",
			"PATH=/sbin:/bin:/usr/sbin:/usr/bin",
			NULL
		      };
static unsigned int nb_batteries = 1;
static char *batteries[] = {"BAT1"};
static void intrpt_routine(struct work_struct *not_used);
static struct delayed_work task;
static DECLARE_DELAYED_WORK(task, intrpt_routine);
static struct workqueue_struct *wq;

static void intrpt_routine(struct work_struct *not_used)
{
	struct power_supply *psy;
	union power_supply_propval capacity;
	unsigned int capacity_length, i;

	for (i = 0; i < nb_batteries; ++i)
	{
		if (battery_args[2] != NULL)
			kfree(battery_args[2]);

		psy = power_supply_get_by_name(batteries[0]);
		if (!power_supply_get_property (psy,
						POWER_SUPPLY_PROP_CAPACITY,
						&capacity))
		{
			capacity_length = capacity.intval<10?
					  	1:
					  capacity.intval<100?
						2:3;
			battery_args[2] = kcalloc(capacity_length+1,
						  sizeof(char),
						  GFP_ATOMIC);
			sprintf(battery_args[2], "%d", capacity.intval);
		}

		call_usermodehelper(APP_PATH, battery_args, envp, UMH_NO_WAIT);
	}

	queue_delayed_work(wq, &task, batIntrptPeriod);
}

static int __init lpc_init (void)
{
	changeParamsFs_init();

	battery_args[1] = kcalloc(5, sizeof(char), GFP_KERNEL);
	sprintf(battery_args[1], "%s", batteries[0]);

	wq = create_workqueue("WQsched.c");
	queue_delayed_work(wq, &task, batIntrptPeriod);

	return 0;
}

static void __exit lpc_exit (void)
{
	changeParamsFs_exit();

	if (battery_args[1] != NULL)
		kfree(battery_args[1]);
	if (battery_args[2] != NULL)
		kfree(battery_args[2]);

	cancel_delayed_work(&task);
	flush_workqueue(wq);
	destroy_workqueue(wq);
}

module_init (lpc_init);
module_exit (lpc_exit);
