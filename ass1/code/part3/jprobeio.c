/* jprobebio.c
   This is a simple module to get information about block io operations.
   Will Cohen
*/

#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/hash.h>
#include <linux/list.h>

#define LOG_2_BINS 8
#define BINS (1<<LOG_2_BINS)
#define numPids (1<<15)

static int count_generic_make_request = 0;
static unsigned long long sectors_transferred = 0;

static struct block_device *bin_bdev[BINS];
static dev_t bin_bd_dev[BINS];
static int bin_count[BINS];
static unsigned long long bin_sectors[BINS];

struct info{
	bool called;
	int nRead;
	int totalRead;
	int nWrite;
	int totalWrite;
	//struct list_head list;
};

struct info infoArr[numPids];

//struct info infoList;
//LIST_HEAD_INIT(&infoList.list);
//LIST_HEAD(infoList);
//INIT_LIST_HEAD(&infoList.list);

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
  	unsigned long b=hash_ptr(bio->bi_bdev, LOG_2_BINS);

  	++count_generic_make_request;
  	sectors_transferred += bio_sectors(bio);
	if((bio->bi_rw & 1) == WRITE)
		populateData(current->pid, 0, 0, 1, bio->bi_iter.bi_size);
	else if((bio->bi_rw & 1) == READ)
		populateData(current->pid, 1, bio->bi_iter.bi_size, 0, 0);
//	printk("Process: %d", current->pid);

	/*
	struct info newinfo = {
		.pid = current->pid,
		.nRead = 0,
		.totalRead = 0,
		.nWrite = 0,
		.totalWrite = 0,
		.list = LIST_HEAD_INIT(newinfo.list),
	};*/
//	struct info* temp = (struct info*)kmalloc(sizeof(struct info));
//	list_add_tail(&newinfo.list, &infoList.list);
	
  	if( bin_bdev[b] == NULL || bin_bdev[b] == bio->bi_bdev) {
	  bin_bdev[b] = bio->bi_bdev;
	  bin_bd_dev[b] = bio->bi_bdev->bd_dev;
	  ++bin_count[b];
	  bin_sectors[b] += bio_sectors(bio);
  	}
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
  int b;
printData();
  unregister_jprobe(&my_jprobe);
  printk("jprobe unregistered\n");
/*
  printk("generic_make_request() called %d times for %lld sectors.\n",
	 count_generic_make_request, sectors_transferred);
  for (b=0; b<BINS; ++b)
    if (bin_bdev[b] != NULL) 
      printk("bdev 0x%p (%d,%d) %d %lld sectors.\n", bin_bdev[b],
	     MAJOR(bin_bd_dev[b]), MINOR(bin_bd_dev[b]),
	     bin_count[b], bin_sectors[b]);*/
}

MODULE_LICENSE("GPL");

