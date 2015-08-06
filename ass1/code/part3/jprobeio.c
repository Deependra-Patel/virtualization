/* jprobebio.c
  Deependra Patel 120050032
*/

#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/list.h>
#define numPids (1<<15)

//Struct used for storing statistic data
struct info{
	bool called;//If process makes a READ/WRITE request
	int nRead;
	int totalRead;
	int nWrite;
	int totalWrite;
};

struct info infoArr[numPids];

void populateData(int pid, int moreRead, int moreReadData, int moreWrite, int moreWriteData){
	if(pid > numPids)
		return;
	infoArr[pid-1].called = true;
	infoArr[pid-1].nRead += moreRead;
	infoArr[pid-1].nWrite += moreWrite;
	infoArr[pid-1].totalRead += moreReadData;
	infoArr[pid-1].totalWrite += moreWriteData;
}

static void inst_generic_make_request(struct bio *bio)
{
	if((bio->bi_rw & 1) == WRITE)
		populateData(current->pid, 0, 0, 1, bio->bi_iter.bi_size);
	else if((bio->bi_rw & 1) == READ)
		populateData(current->pid, 1, bio->bi_iter.bi_size, 0, 0);
  	jprobe_return();
}

static struct jprobe my_jprobe = {
	.kp.addr = (kprobe_opcode_t *) generic_make_request,
	.entry = (kprobe_opcode_t *) inst_generic_make_request
};

int init_module(void)
{
	register_jprobe(&my_jprobe);
	printk("plant jprobe at %p, handler addr %p\n",
	       my_jprobe.kp.addr, my_jprobe.entry);
	return 0;
}

void printData(void){
	int i;	
	for(i = 0; i<numPids; i++){
		if(infoArr[i].called){
			printk("PID: %d, No. of Reads: %d, Total Data Read %d, No. of Writes: %d, Total Data Written %d\n", i+1, infoArr[i].nRead, infoArr[i].totalRead, infoArr[i].nWrite, infoArr[i].totalWrite);		
		}
	}
}

void cleanup_module(void)
{
	printData();
  	unregister_jprobe(&my_jprobe);
	printk("jprobe unregistered\n");
}


