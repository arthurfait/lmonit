#ifndef INFO_H_
#define INFO_H_

#include <time.h>
#include <stdint.h>

// enums needed
typedef enum _status  {
    fail = 0,
    ok = 1
} status_t;

typedef struct part_node {
    uint32_t blocks;
    char name[8];
} part_node;

typedef struct part_stat {
    int num_nodes;
    struct part_node* node;
} part_stat;


struct cpu_loads {
    unsigned long int u;
    unsigned long int n;
    unsigned long int s;
    unsigned long int l;
};

// struct to save info of the general;
struct mem_info {
    uint32_t memtotal;
    uint32_t memfree;
    uint32_t memavailable;
    uint32_t buffers;
    uint32_t cashed;
    uint32_t swaptotal;
    uint32_t swapfree;
};

// known bug(feature :-) ) supports only first processor if there are two or more processors
struct cpuinfo {
    char name[255];
    uint32_t mhz;
    uint32_t cache;
};

struct info {
    double load;
    struct mem_info mi;
    uint32_t uptime;
};



typedef struct process {
    int pid;
    char* user_name;
    double cpu;
    double mem;
    char* cdm_str;
} process;

typedef struct process_list {
    int num_nodes;
    struct process* procs;
    char init;
} process_list;

typedef struct mnts {
    char* fs_dev;
    char* type;
    uint32_t blocks;
    uint32_t blocks_used;
    uint32_t blocks_available;
    uint32_t use;
    char* mnt_to;

} mnts;

typedef struct mounts_list {
    int num_nodes;
    mnts* mounts;
    char inst;
} mounts_list;

void get_fir(void);
void get_sec(struct info* inf);
void get_cpu_info(struct cpuinfo* ci);
status_t get_part_stat(part_stat* partition);
status_t get_p_stat(process_list* pl, int count, char* username);
void destr(process_list* pl);
status_t get_mounts_stat(mounts_list* dl, int count);
void destr_mounts(mounts_list* pl);
#endif /* INFO_H_ */
