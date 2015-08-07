/* part4.c
  Deependra Patel 120050032
*/

#include <linux/module.h>
#include <linux/sched.h>

int pid;
module_param(pid, int, 0);

int init_module(void)
{
  printk("Inserted module.\n");
	return 0;
}

void printData(void){
	struct task_struct *task = pid_task(find_vpid(pid), PIDTYPE_PID);
	
	printk("About process with pid: %d\n", pid);
	printk("Parent pid: %d \n", task->real_parent->pid); 
	if(task->state == -1)
	  printk("Status: Process is unrunnable\n");
	else if(task->state == 0)
	  printk("Status: Process is runnable\n");
	else if(task->state>0)
	  printk("Status: Process is stopped\n");
	printk("RT Priority: %d\n", task->rt_priority);
}

void cleanup_module(void)
{
	printData();
	printk("Cleaning up module.\n");
}
MODULE_LICENSE("GPL");

