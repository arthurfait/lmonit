/*
 * info.c
 *
 *  Created on: 19.12.2011
 *      Author: arthur
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <string.h>
#include "info.h"
#include <panel.h>

struct cpu_loads cp_1 = {0, 0, 0, 0};

void get_fir(void)
{
    FILE* stat = fopen("/proc/stat", "r");
    if (!stat) {
        endwin();
        exit(0);
    }

    fscanf(stat, "cpu %lu %lu %lu %lu",
           &cp_1.u, &cp_1.n, &cp_1.s, &cp_1.l);
    fclose(stat);
}

void get_sec(struct info* inf)
{
    struct cpu_loads cp_2;
    int total;
    char dumy[255];
    double cpu, cps, cpn, cp;
    FILE* stat = fopen("/proc/stat", "r");
    if (!stat) {
        endwin();
        exit(0);
    }
    fscanf(stat, "cpu %lu %lu %lu %lu", &cp_2.u, &cp_2.n, &cp_2.s, &cp_2.l);
    cp_2.u -= cp_1.u;
    cp_2.n -= cp_1.n;
    cp_2.s -= cp_1.s;
    cp_2.l -= cp_1.l;
    total = cp_2.u + cp_2.n + cp_2.s + cp_2.l;
    cpu = (double)cp_2.u / (double)total;
    cps = (double)cp_2.s / (double)total;
    cpn = (double)cp_2.n / (double)total;
    cp = cpu + cps + cpn;
    fclose(stat);
    FILE* mem = fopen("/proc/meminfo", "r");
    if (!mem) {
        endwin();
        exit(0);
    }
    inf->load = cp;
    // FIXME: reading shall not be positional file fmt can change
    fscanf(mem, "MemTotal:%u kB\n", &inf->mi.memtotal);
    fscanf(mem, "MemFree:%u kB\n", &inf->mi.memfree);
    fscanf(mem, "MemAvailable:%u kB\n", &inf->mi.memavailable);
    fscanf(mem, "Buffers:%u kB\n", &inf->mi.buffers);
    fscanf(mem, "Cached:%u kB\n", &inf->mi.cashed);
    while (strcmp(dumy, "SwapTotal:") != 0) {
        fscanf(mem, "%s %u kB\n", dumy, &inf->mi.swaptotal);
    }
    fscanf(mem, "SwapFree:%u kB", &inf->mi.swapfree);
    fclose(mem);
    FILE* uptm = fopen("/proc/uptime", "r");
    if (!uptm) {
        endwin();
        exit(0);
    }
    fscanf(uptm, "%u", &inf->uptime);
    fclose(uptm);


}

void get_cpu_info(struct cpuinfo* ci)
{
    FILE* cpu_file = fopen("/proc/cpuinfo", "r");
    if (!cpu_file) {
        endwin();
        fprintf(stderr, "Error\n");
        exit(0);
    }
    char na[255], model_str[255], *str;
    fgets(na, 255, cpu_file);
    fgets(na, 255, cpu_file);
    fgets(na, 255, cpu_file);
    fgets(na, 255, cpu_file);
    fgets(model_str, 255, cpu_file);
    str = strchr(model_str, ':');
    str++;
    strcpy(ci->name, str);
    fgets(na, 255, cpu_file);
    fgets(na, 255, cpu_file);
    fscanf(cpu_file, "cpu MHz		:%u\n", &ci->mhz);
    fscanf(cpu_file, "cache size	: %u KB", &ci->cache);
    fclose(cpu_file);
}

status_t get_part_stat(part_stat* partition)
{
    char dumy[80];
    int dum;
    FILE* part_file = fopen("/proc/partitions", "r");
    int num = 0, i = 0;
    if (!part_file) {
        return fail;
    }
    while (!feof(part_file)) {
        fgets(dumy, 80, part_file);
        num++;
    }
    num -= 3;
    partition->node = (part_node*) malloc(sizeof(part_node) * num);
    partition->num_nodes = num;
    fseek(part_file, 0, SEEK_SET);
    fgets(dumy, 80, part_file);
    fgets(dumy, 80, part_file);
    while (!feof(part_file)) {
        unsigned int size;
        char name[8];
        fscanf(part_file, "%d %d %u %s\n", &dum, &dum, &size, name);
        partition->node[i].blocks = size;
        strcpy(partition->node[i].name, name);
        i++;
    }
    fclose(part_file);
    return ok;
}

status_t get_p_stat(process_list* pl, int count, char* user_name)
{
    char str[1024];
    int realy = 0;
    double cpu, mem;
    char name[80];
    char cdm_str[256];
    int pid;
    int i = 0;
    int len;
    FILE* prs = popen("ps -eo pid,user,%cpu,%mem,cmd", "r");
    if (!prs) {
        return fail;
    }
    int skip = 0;
    if (user_name)
        skip = 1;
    pl->num_nodes = count;
    pl->procs = (process*) malloc(sizeof(process) * count);
    fgets(str, 1024, prs);
    while (!feof(prs)) {
        fgets(str, 1024, prs);
        sscanf(str, "%d %s %lf %lf %s", &pid, name, &cpu, &mem, cdm_str);
        if (skip && (strcmp(name, user_name))) goto sk;
        pl->procs[i].pid = pid;
        len = strlen(name);
        pl->procs[i].user_name  = calloc(len, 1);
        strcpy(pl->procs[i].user_name, name);
        pl->procs[i].cpu = cpu;
        pl->procs[i].mem = mem;
        len = strlen(cdm_str);
        pl->procs[i].cdm_str =  calloc(len + 1, 1);
        strcpy(pl->procs[i].cdm_str, cdm_str);
        realy = i;
        i++;


        if (i > count) {
            break;
        }
    sk:
        ;
    }
    pl->num_nodes = realy;
    pl->init = 1;
    pclose(prs);

    return ok;
}

void destr(process_list* pl)
{
    int i;
    for (i = 0; i < pl->num_nodes; i++) {
        free(pl->procs[i].cdm_str);
        free(pl->procs[i].user_name);
    }
    free(pl->procs);
    pl->init = 0;
}


status_t get_mounts_stat(mounts_list* dl, int count)
{
    char str[512];
    char fs_dev[80];
    char type[80];
    uint32_t blocks;
    uint32_t blocks_used;
    uint32_t blocks_available;
    uint32_t use;
    char mnt_to[256];
    int i = 0;
    int len;
    int realy = 0;
    FILE* prs = popen("df -T", "r");
    if (!prs) {
        return fail;
    }
    dl->mounts = (mnts*) malloc(sizeof(mnts) * count);

    dl->num_nodes = count;
    i = 0;
    fgets(str, 512, prs);
    while (!feof(prs)) {
        fgets(str, 512, prs);
        sscanf(str, "%s %s %u %u %u %u%% %s\n", fs_dev, type, &blocks, &blocks_used, &blocks_available, &use, mnt_to);
        // skip if none
        if (strcmp(fs_dev, "none")) {
            len = strlen(fs_dev);
            dl->mounts[i].fs_dev = calloc(len + 1, 1);

            strcpy(dl->mounts[i].fs_dev, fs_dev);
            len = strlen(type);
            dl->mounts[i].type = calloc(len + 1, 1);

            strcpy(dl->mounts[i].type, type);

            len = strlen(mnt_to);
            dl->mounts[i].mnt_to = calloc(len + 1, 1);
            strcpy(dl->mounts[i].mnt_to, mnt_to);

            dl->mounts[i].blocks = blocks;
            dl->mounts[i].blocks_available = blocks_available;
            dl->mounts[i].blocks_used = blocks_used;
            dl->mounts[i].use = use;
            realy = i;
            i++;

            if (i > count) {
                break;
            }
        }
    }
    dl->num_nodes = realy;
    dl->inst = 1;
    pclose(prs);
    return ok;
}

void destr_mounts(mounts_list* pl)
{
    int i;
    if (pl->mounts) {
        for (i = 0; i < pl->num_nodes; i++) {
            if (pl->mounts[i].fs_dev)
                free(pl->mounts[i].fs_dev);
            if (pl->mounts[i].mnt_to)
                free(pl->mounts[i].mnt_to);
            if (pl->mounts[i].mnt_to)
                free(pl->mounts[i].type);
        }
        free(pl->mounts);
    }
}

