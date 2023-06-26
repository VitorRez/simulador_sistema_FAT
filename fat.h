#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <inttypes.h>

#define SECTOR_SIZE	     512
#define CLUSTER_SIZE     2*SECTOR_SIZE
#define ENTRY_BY_CLUSTER CLUSTER_SIZE/sizeof(dir_entry_t)
#define NUM_CLUSTER	     4096
#define fat_name	     "fat.part"

struct _dir_entry_t {
	char filename[18];
	uint8_t attributes;
	uint8_t reserved[7];
	uint16_t first_block;
	uint32_t size;
};

typedef struct _dir_entry_t  dir_entry_t;

union _data_cluster {
	dir_entry_t dir[ENTRY_BY_CLUSTER];
	uint8_t data[CLUSTER_SIZE];
};

typedef union _data_cluster data_cluster;

void init(uint16_t*, uint8_t*, dir_entry_t*, data_cluster*);
void load(uint8_t*, uint16_t*, dir_entry_t*, data_cluster*);
void ls(data_cluster*);
void mkdir(uint16_t*, char*, data_cluster*);
void create(uint16_t*, char*, data_cluster*);
void unlink(uint16_t*, char*, data_cluster*);
void write(uint16_t*, char*, char*, int, data_cluster*);
void append(uint16_t*, char*, char*, int, data_cluster*);
void read(uint16_t*, char*, data_cluster*);
void save(uint16_t*, uint8_t*, dir_entry_t*, data_cluster*);
void search_in_fat(uint16_t*, data_cluster*, int);
void rm_dir_in_fat(uint16_t*, data_cluster*, int);
