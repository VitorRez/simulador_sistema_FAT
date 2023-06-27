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

int cd(data_cluster *clusters, char* dirname){
	//printf("%s\n", dirname);
	for(int i = 1; i < 4086; i++){
		if(strcmp(clusters[i].dir->filename, dirname) == 0){
			//printf("%s %d\n", clusters[i].dir->filename, i);
			return i;
		}
	}
}

void ls(data_cluster *clusters, int current_dir) {
	for(int i = 1; i < 4086; i++){
		if(strlen(clusters[i].dir->filename) > 0 /*&& clusters[i].dir->attributes == 1*/ && clusters[i].dir->parent_dir == current_dir){
			printf("%s ", clusters[i].dir->filename);
		}
	}
	printf("\n");
}

void mkdir(uint16_t *fat, char* dirname, data_cluster *clusters, int current_dir) {
	for(int i = 1; i < 4086; i++){
		if(strlen(clusters[i].dir->filename) == 0){
			strcpy(clusters[i].dir->filename, dirname);
			clusters[i].dir->attributes = 1;
			clusters[i].dir->parent_dir = current_dir;
			clusters[i].dir->first_block = i;
			fat[10 + i] = i;
			return;
		}
	}
}

void create(uint16_t *fat, char* filename, data_cluster *clusters, int current_dir){
	for(int i = 1; i < 4086; i++){
		if(strlen(clusters[i].dir->filename) == 0){
			strcpy(clusters[i].dir->filename, filename);
			clusters[i].dir->attributes = 0;
			clusters[i].dir->size = 36;
			clusters[i].dir->parent_dir = current_dir;
			clusters[i].dir->first_block = i;
			fat[10 + i] = i;
			return;
		}
	}
}

void unlink(uint16_t *fat, char* name, data_cluster *clusters, int current_dir){
	for(int i = 1; i < 4086; i++){
		if(strcmp(clusters[i].dir->filename, name) == 0 && clusters[i].dir->parent_dir == current_dir){
			if(clusters[i].dir->attributes == 1){
				for(int j = i; j < 4086; j++){
					if(clusters[j].dir->parent_dir == i){
						return;
					}
				}
			}
			if(fat[10 + i] == i){
				strcpy(clusters[i].dir->filename, "");
				clusters[i].dir->attributes = -1;
				clusters[i].dir->parent_dir = 0;
				rm_dir_in_fat(fat, clusters, 10 + i);
			}else{
				int x = fat[10 + i];
				strcpy(clusters[i].dir->filename, "");
				clusters[i].dir->attributes = -1;
				clusters[i].dir->parent_dir = 0;
				rm_dir_in_fat(fat, clusters, 10 + i);
				do{
					strcpy(clusters[x].dir->filename, "");
					clusters[x].dir->attributes = -1;
					clusters[i].dir->parent_dir = 0;
					x = fat[10 + x];
					rm_dir_in_fat(fat, clusters, 10 + x);
				}while(x != -1);
			}
			return;
		}
	}
}

void write(uint16_t *fat, char* filename, char* text, int size, data_cluster *clusters, int current_dir){
	int i;
	int j;	
	for(i = 1; i < 4086; i++){
		if(strcmp(clusters[i].dir->filename, filename) == 0 && clusters[i].dir->attributes == 0){
			if(size > 1024 - 36){
				clusters[i].dir->size = 36;
				for(j = 0; j < size; j++){
					if(j == 1024 - 36) break;
					clusters[i].data[j+36] = (int)text[j];
				}
				clusters[i].dir->size = 1024;
				int x = size - 1024 - 36;
				int fat_now = clusters[i].dir->first_block;
				do{
					for(int k = i; k < 4086; k++){
						if(strlen(clusters[k].dir->filename) == 0){
							strcpy(clusters[k].dir->filename, filename);
							clusters[k].dir->attributes = 0;
							clusters[k].dir->size = 36;
							clusters[k].dir->parent_dir = current_dir;
							for(int l = 0; l < size; l++){
								if(l == 1024 - 36) break;
								clusters[k].data[l + 1024 + 36] = (int)text[l];
							}
							fat[10 + fat_now] = k;
							fat_now = k;
							break;
						}
					}
					x = size - 1024 - 36;
				}while(x < 1024);
				fat[10 + fat_now] = -1;
			}else{
				clusters[i].dir->size = 36;
				for(j = 0; j < size; j++){
					clusters[i].data[j+36] = (int)text[j];
				}
				clusters[i].dir->size = 36 + size;
				//search_in_fat(fat, clusters, i);
				return;
			}
		}
	}
}

void append(uint16_t *fat, char* filename, char* text, int size, data_cluster *clusters, int current_dir){
	int i;
	int j;
	for(i = 1; i < 4086; i++){
		if(strcmp(clusters[i].dir->filename, filename) == 0 && clusters[i].dir->attributes == 0 && clusters[i].dir->parent_dir == current_dir){
			if(clusters[i].dir->size + size > 1024 - 36){
				for(j = 0; j < size; j++){
					if(j == 1024 - 36) break;
					clusters[i].data[j+clusters[i].dir->size] = (int)text[j];
				}
				clusters[i].dir->size = 1024;
				int x = size - 1024 - 36;
				int fat_now = find_last_block(fat, clusters[i].dir->first_block);
				do{
					for(int k = i; k < 4086; k++){
						if(strlen(clusters[k].dir->filename) == 0){
							strcpy(clusters[k].dir->filename, filename);
							clusters[k].dir->attributes = 0;
							clusters[k].dir->size = 36;
							clusters[k].dir->parent_dir = current_dir;
							for(int l = 0; l < size; l++){
								if(l == 1024 - 36) break;
								clusters[k].data[l + 1024 + 36] = (int)text[l];
							}
							fat[10 + fat_now] = k;
							fat_now = k;
							break;
						}
					}
					x = size - 1024 - 36;
				}while(x < 1024);
				fat[10 + fat_now] = -1;
			}
			else{
				for(j = 0; j < size; j++){
					clusters[i].data[j+clusters[i].dir->size] = (int)text[j];
				}
				clusters[i].dir->size = clusters[i].dir->size + size;
			}
			//search_in_fat(fat, clusters, i);
			return;
		}
	}
}

void read(uint16_t *fat, char* filename, data_cluster *clusters, int current_dir){
	int i = 0;
	uint8_t j = 36;
	for(i = 1; i < 4086; i++){
		if(strcmp(clusters[i].dir->filename, filename) == 0 && clusters[i].dir->attributes == 0 && clusters[i].dir->parent_dir == current_dir){
			if(fat[10 + i] == i){
				for(j = 36; j <= clusters[i].dir->size; j++){
					printf("%c", clusters[i].data[j]);
				}
				printf("\n");
			}else{
				for(j = 36; j <= clusters[i].dir->size; j++){
					printf("%c", clusters[i].data[j]);
				}
				int x = fat[10 + i];
				do{
					for(j = 36; j <= clusters[x].dir->size; j++){
						printf("%c", clusters[x].data[j]);
					}
					x = fat[10 + x];
				}while(x != -1);
			printf("\n");
			}
		}
	}
}

int find_last_block(uint16_t *fat, int first_block){
	int x = fat[first_block];
	while(x != -1){
		x = fat[10 + x];
	}
	return x;
}

void rm_dir_in_fat(uint16_t *fat, data_cluster* clusters, int i){
	fat[clusters[i].dir->first_block] = 0x0000;
}


