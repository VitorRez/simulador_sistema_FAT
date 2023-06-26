#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fat.h"

char** processar_string(char* text, char* separador, int cont){
    char** mat = (char**)malloc(cont*sizeof(char*));
    char str[100];
    for(int i = 0; i < cont; i++){
        mat[i] = str;
    }
    char* token = strtok(text, separador);
    cont = 0;
    while(token != NULL){
        mat[cont] = token;
        token = strtok(NULL, separador);
        cont++;
    }
    return mat;
}

int get_num_lines(char* text){
    int cont = 1;
    for(int i = 0; i < strlen(text); i++){
        if(text[i] == ' '){
            cont++;
        }
    }
    return cont;
} 

void get_command(char* command, uint16_t* fat, uint8_t* boot_block, dir_entry_t* root_dir, data_cluster* clusters){
    int num_linhas = get_num_lines(command);
    char** commands = processar_string(command, " ", num_linhas);
    if(strcmp(commands[0], "init") == 0){
        init(fat, boot_block, root_dir, clusters);
    }
    if(strcmp(commands[0], "load") == 0){
        load(boot_block, fat, root_dir, clusters);
    }
    if(strcmp(commands[0], "ls") == 0){
        ls(clusters);
    }
    if(strcmp(commands[0], "mkdir") == 0){
        mkdir(fat, commands[1], clusters);
        save(fat, boot_block, root_dir, clusters);
    }
    if(strcmp(commands[0], "create") == 0){
        create(fat, commands[1], clusters);
        save(fat, boot_block, root_dir, clusters);
    }
    if(strcmp(commands[0], "unlink") == 0){
        unlink(fat, commands[1], clusters);
        save(fat, boot_block, root_dir, clusters);
    }
    if(strcmp(commands[0], "write") == 0){
        char text[256];
        for(int i = 2; i < num_linhas; i++){
            commands[i][strcspn(commands[i], " ")] = 0; 
            strcat(text, commands[i]);
            if(i < num_linhas-1) strcat(text, " ");
        }
        // printf("%s\n", text);
        int size = strlen(text);
        write(fat, commands[1], text, size, clusters);
        save(fat, boot_block, root_dir, clusters);
    }
    if(strcmp(commands[0], "append") == 0){
        char text[256];
        for(int i = 2; i < num_linhas; i++){ 
            commands[i][strcspn(commands[i], " ")] = 0; 
            strcat(text, commands[i]);
            if(i < num_linhas-1) strcat(text, " ");
        }
        // printf("%s\n", text);
        int size = strlen(text);
        append(fat, commands[1], text, size, clusters);
        save(fat, boot_block, root_dir, clusters);
    }
    if(strcmp(commands[0], "read") == 0){
        read(fat, commands[1], clusters);
    }
}

int main(){
    /*DATA DECLARATION*/
    static uint16_t fat[NUM_CLUSTER];
    static uint8_t boot_block[CLUSTER_SIZE];
    static dir_entry_t root_dir[ENTRY_BY_CLUSTER];
    static data_cluster clusters[4086];
    char* prompt = ">>Diga la:";
    char text[512];
    do{
        printf("%s ", prompt);
        if(fgets(text, 100, stdin) == NULL) break;
        text[strcspn(text, "\n")] = 0;
        get_command(text, fat, boot_block, root_dir, clusters);
        if(strcmp(text, "fim") == 0) break;
    }while(strcmp(text, "fim\n") != 0 || (text[0] = getchar()) == EOF);
    return 0;
}