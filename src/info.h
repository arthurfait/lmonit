#ifndef INFO_H_
#define INFO_H_

#include <time.h>

// enums needed
typedef enum status  {fail=0,ok=1} status;
typedef unsigned int uint;

typedef struct part_node{
	uint blocks;
	char name[8];
} part_node;

typedef struct part_stat{
	int num_nodes;
	struct part_node* node;
} part_stat;


struct cpu_loads{
	unsigned long int u;
	unsigned long int n;
	unsigned long int s;
	unsigned long int l;
};

// struct to save info of the general;
struct mem_info {
	uint memtotal;
	uint memfree;
	uint buffers;
	uint cashed;
	uint swaptotal;
	uint swapfree;
};

// known bug(feature :-) ) supports only first processor if there are two or more processors
struct cpuinfo{
	char name[255];
	uint mhz;
	uint cache;
};

struct info{
	double load;
	struct mem_info mi;
	uint uptime;
};



typedef struct process{
	int pid;
	char *user_name;
	double cpu;
	double mem;
	char *cdm_str;
} process;

typedef struct process_list{
	int num_nodes;
	struct process* procs;
	char init;
} process_list;

typedef struct mnts{
	char *fs_dev;
	char *type;
	uint blocks;
	uint blocks_used;
	uint blocks_available;
	uint use;
	char *mnt_to;

} mnts;

typedef struct mounts_list{
	int num_nodes;
	mnts* mounts;
	char inst;
} mounts_list;

void get_fir(void);
void get_sec(struct info *inf);
void get_cpu_info(struct cpuinfo *ci);
status get_part_stat(part_stat* partition);
status get_p_stat(process_list* pl,int count,char *username);
void destr(process_list *pl);
status get_mounts_stat(mounts_list* dl,int count);
void destr_mounts(mounts_list *pl);
#endif /* INFO_H_ */
