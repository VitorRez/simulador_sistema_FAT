#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fat.h"

void init(uint16_t *fat, uint8_t *boot_block, dir_entry_t *root_dir, data_cluster *clusters) {
	FILE* ptr_file;
	int i;
	ptr_file = fopen(fat_name,"wb");
	for (i = 0; i < CLUSTER_SIZE; ++i)
		boot_block[i] = 0xbb;

	fwrite(&boot_block, sizeof(boot_block), 1,ptr_file);

	fat[0] = 0xfffd;
	for (i = 1; i < 9; ++i)
		fat[i] = 0xfffe;

	fat[9] = 0xffff;
	for (i = 10; i < NUM_CLUSTER; ++i)
		fat[i] = 0x0000;

	fwrite(&fat, sizeof(fat), 1, ptr_file);
	
    memset(root_dir, 0, sizeof(root_dir));
    fwrite(&root_dir, sizeof(root_dir), 1,ptr_file);

    memset(clusters, 0, sizeof(clusters));
	fwrite(&clusters, sizeof(clusters), 1, ptr_file);

	fclose(ptr_file);
}

void save(uint16_t *fat, uint8_t *boot_block, dir_entry_t *root_dir, data_cluster *clusters) {
	FILE* ptr_file;
	ptr_file = fopen(fat_name,"wb");
	fwrite(boot_block, sizeof(boot_block), CLUSTER_SIZE, ptr_file);
	fwrite(fat, sizeof(fat), NUM_CLUSTER, ptr_file);
    fwrite(root_dir, sizeof(root_dir), ENTRY_BY_CLUSTER, ptr_file);
	fwrite(clusters, sizeof(clusters), 4086, ptr_file);
	fclose(ptr_file);
}

void load(uint8_t *boot_block, uint16_t *fat, dir_entry_t *root_dir, data_cluster *clusters) {
	FILE* ptr_file;
	ptr_file = fopen(fat_name, "rb");
	fread(boot_block, sizeof(boot_block), CLUSTER_SIZE, ptr_file);
	fread(fat, sizeof(fat), NUM_CLUSTER, ptr_file);
	fread(root_dir, sizeof(root_dir), ENTRY_BY_CLUSTER, ptr_file);
	fread(clusters, sizeof(clusters), 4086, ptr_file);
	fclose(ptr_file);
}

void ls(data_cluster *clusters) {
	for(int i = 1; i < 4086; i++){
		if(strlen(clusters[i].dir->filename) > 0 && clusters[i].dir->attributes == 1){
			printf("%s ", clusters[i].dir->filename);
		}
	}
	printf("\n");
}

void mkdir(uint16_t *fat, char* dirname, data_cluster *clusters) {
	for(int i = 1; i < 4086; i++){
		if(strlen(clusters[i].dir->filename) == 0){
			strcpy(clusters[i].dir->filename, dirname);
			clusters[i].dir->attributes = 1;
			search_in_fat(fat, clusters, i);
			return;
		}
	}
}

void create(uint16_t *fat, char* filename, data_cluster *clusters){
	for(int i = 1; i < 4086; i++){
		if(strlen(clusters[i].dir->filename) == 0){
			strcpy(clusters[i].dir->filename, filename);
			clusters[i].dir->attributes = 0;
			clusters[i].dir->size = 32;
			search_in_fat(fat, clusters, i);
			return;
		}
	}
}

void unlink(uint16_t *fat, char* name, data_cluster *clusters){
	for(int i = 1; i < 4086; i++){
		if(strcmp(clusters[i].dir->filename, name) == 0){
			strcpy(clusters[i].dir->filename, "");
			clusters[i].dir->attributes = -1;
			rm_dir_in_fat(fat, clusters, i);
			return;
		}
	}
}

void write(uint16_t *fat, char* filename, char* text, int size, data_cluster *clusters){
	int i;
	int j;	
	for(i = 1; i < 4086; i++){
		if(strcmp(clusters[i].dir->filename, filename) == 0 && clusters[i].dir->attributes == 0){
			clusters[i].dir->size = 32;
			for(j = 0; j < size; j++){
				clusters[i].data[j+32] = (int)text[j];
			}
			clusters[i].dir->size = 32 + size;
			search_in_fat(fat, clusters, i);
			return;
		}
	}
}

void append(uint16_t *fat, char* filename, char* text, int size, data_cluster *clusters){
	int i;
	int j;
	for(i = 1; i < 4086; i++){
		if(strcmp(clusters[i].dir->filename, filename) == 0 && clusters[i].dir->attributes == 0){
			for(j = 0; j < size; j++){
				clusters[i].data[j+clusters[i].dir->size] = (int)text[j];
			}
			clusters[i].dir->size = clusters[i].dir->size + size;
			search_in_fat(fat, clusters, i);
			return;
		}
	}
}

void read(uint16_t *fat, char* filename, data_cluster *clusters){
	int i = 0;
	uint8_t j = 32;
	for(i = 1; i < 4086; i++){
		if(strcmp(clusters[i].dir->filename, filename) == 0 && clusters[i].dir->attributes == 0){
			for(j = 32; j <= clusters[i].dir->size; j++){
				printf("%c", clusters[i].data[j]);
			}
			printf("\n");
		}
	}
}

void search_in_fat(uint16_t *fat, data_cluster *clusters, int i){
	for(int j = 11; j < 4096; j++){
		if(fat[j] == 0x0000){
			//printf("bosta %d\n", j);
			clusters[i].dir->first_block = j;
			fat[j] = j;
			return;
		}
	}
}

void rm_dir_in_fat(uint16_t *fat, data_cluster* clusters, int i){
	fat[clusters[i].dir->first_block] = 0x0000;
}

void update_fat(uint16_t *fat, data_cluster *clusters, int i){
	for(int j = 11; j < NUM_CLUSTER; j++){
		if(fat[j] == 0x0000){
			fat[clusters[i].dir->first_block] = j;
			fat[j] = j;
		}
	}
}

