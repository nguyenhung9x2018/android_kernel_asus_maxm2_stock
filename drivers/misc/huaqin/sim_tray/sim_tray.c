/*
 * file create by liunianliang for check sim tray status, 2018/07/05
 *
 * 2018.07.31
 * Modify By LiHua For ZQL1830-235
 * Description: config interrupt gpio, gpio54/gpio58/gpio133 has the same function on project ZQL1830.
 */

#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include <linux/utsname.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <asm/uaccess.h>
#include <linux/printk.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/seq_file.h>

#define SIM_TRAY_HOT_DETECH_GPIO 54 // should modify this GPIO according to project.

static int sim_tray_status_show(struct seq_file *m, void *v)
{
	seq_printf(m, "STATUS=%d\n", gpio_get_value(SIM_TRAY_HOT_DETECH_GPIO));
	return 0;
}

static int sim_tray_file_open(struct inode *inode, struct file *file)
{
	return single_open(file, sim_tray_status_show, inode->i_private);
}

static const struct file_operations sim_tray_fops = {
	.open = sim_tray_file_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __init sim_tray_init(void)
{
	struct proc_dir_entry *ret;

	ret = proc_mkdir("class", NULL);
	if(!ret) {
		pr_err("init proc/class failed!!!\n");
		return -ENOMEM;
	}

	ret = proc_mkdir("sd-hotplug", ret);
	if(!ret) {
		pr_err("init proc/class/sd-hotplug failed!!!\n");
		return -ENOMEM;
	}

	ret = proc_mkdir("status", ret);
	if(!ret) {
		pr_err("init proc/class/sd-hotplug/status failed!!!\n");
		return -ENOMEM;
	}

	ret = proc_create("hot-status", 0444, ret, &sim_tray_fops);

	return 0;
}

late_initcall(sim_tray_init);
