#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/blkdev.h>

static int count_generic_make_request = 0;

static int inst_generic_make_request(struct kprobe *p, struct pt_regs *regs)
{
	++count_generic_make_request;
	printk("pre_handler: p->addr=0x%p\n",p->addr);
	return 0;
}

/*For each probe you need to allocate a kprobe structure*/
static struct kprobe kp = {
	.pre_handler = inst_generic_make_request,
	.post_handler = NULL,
	.fault_handler = NULL,
	.addr = (kprobe_opcode_t *) generic_make_request,
};

int init_module(void)
{
	register_kprobe(&kp);
	printk("kprobe registered\n");
	return 0;
}

void cleanup_module(void)
{
  unregister_kprobe(&kp);
  printk("kprobe unregistered\n");
  printk("generic_make_request() called %d times.\n",
	 count_generic_make_request);
}

MODULE_LICENSE("GPL");
