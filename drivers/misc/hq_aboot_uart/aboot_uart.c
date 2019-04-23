/*
 * file create by liunianliang for ZQL1626-1175,2017.12.11
 */

#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include <linux/utsname.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <asm/uaccess.h>
#include <linux/printk.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/seq_file.h>

static char *aboot_add_buf;
extern char *saved_command_line;

#define PAGE_BUF_SIZE   1024
#define UART_LOG_TOTAL_SIZE   0x7c00    /* 0x7c00 is 31k, refers to aboot */

static ssize_t
hq_aboot_uart_write(struct file *filp, const char *ubuf, size_t cnt, loff_t *data)
{
	return cnt;
}

static int hq_aboot_uart_show(struct seq_file *m, void *v)
{
	unsigned long uart_start_phy_addr, uart_cnt_phy_addr;
	void * uart_start_virt_addr;
	void * uart_cnt_virt_addr;
	char buf[PAGE_BUF_SIZE], cnt[8];
	unsigned long count;
	int page = 0;

	uart_start_phy_addr = simple_strtoul(aboot_add_buf, NULL, 0);
	uart_cnt_phy_addr = uart_start_phy_addr + UART_LOG_TOTAL_SIZE;
	uart_start_virt_addr = phys_to_virt(uart_start_phy_addr);
	uart_cnt_virt_addr = phys_to_virt(uart_cnt_phy_addr);
	memcpy(cnt, uart_cnt_virt_addr, sizeof(cnt));
	count = simple_strtoul(cnt, NULL, 0);

	seq_printf(m, "total buf count = %lu\n", count);
	seq_printf(m, "\n>>>>>>>>>>>>>>> lk uart log <<<<<<<<<<<<<<<\n\n");
	for (page = 0; page <= (int)(count/PAGE_BUF_SIZE); page++) {
		if (page == (int)(count/PAGE_BUF_SIZE)) {
			memcpy(buf, uart_start_virt_addr + page * PAGE_BUF_SIZE,
				(int)(count%PAGE_BUF_SIZE));
			buf[count%PAGE_BUF_SIZE -1] = '\0';
			seq_printf(m, "%s\n", buf);
		} else {
			memcpy(buf, uart_start_virt_addr + page * PAGE_BUF_SIZE, PAGE_BUF_SIZE);
			buf[PAGE_BUF_SIZE -1] = '\0';
			seq_printf(m, "%s", buf);
		}
	}
	return 0;
}

/*** Seq operation of mtprof ****/
static int hq_aboot_uart_open(struct inode *inode, struct file *file)
{
	return single_open(file, hq_aboot_uart_show, inode->i_private);
}

static const struct file_operations hq_aboot_uart_fops = {
	.open = hq_aboot_uart_open,
	.write = hq_aboot_uart_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __init init_aboot_uart(void)
{
	struct proc_dir_entry *ret;
	char *br_ptr;
	aboot_add_buf = kzalloc(11, GFP_ATOMIC | __GFP_NORETRY |
			  __GFP_NOWARN);
	pr_info("init_aboot_uart: enter !!!\n");
	br_ptr = strstr(saved_command_line, "aboot_uart_add=");
	if (br_ptr != 0) {
		sscanf(br_ptr, "aboot_uart_add=%s ", aboot_add_buf);
		pr_info("init_aboot_uart: %s\n", aboot_add_buf);
	}

	ret = proc_create("aboot_uart", 0444, NULL, &hq_aboot_uart_fops);
	if (!ret) {
		pr_err("init_aboot_uart: create proc/aboot_uart failed !!\n");
		return -ENOMEM;
	}
	return 0;
}

device_initcall(init_aboot_uart);
