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

int get_num_lines(char* text, char separador){
    int cont = 1;
    for(int i = 0; i < strlen(text); i++){
        if(text[i] == separador){
            cont++;
        }
    }
    return cont;
} 

int get_command(char* command, uint16_t* fat, uint8_t* boot_block, dir_entry_t* root_dir, data_cluster* clusters, int current_dir){
    int num_linhas = get_num_lines(command, ' ');
    char** commands = processar_string(command, " ", num_linhas);
    /*int num_dirs = get_num_lines(command, '/') - 1;
    char **path = processar_string(command, "/", num_dirs);
    for(int i = 0; i < num_dirs; i++){
        printf("%s ", path[i]);
    }*/
    if(strcmp(commands[0], "cd") == 0){
        if(commands[1] == NULL){
            current_dir = 0;
            return current_dir;
        }
        if(commands[1][0] != '/') return current_dir;
        int num_dirs = get_num_lines(commands[1], '/') - 1;
        char **path = processar_string(commands[1], "/", num_dirs);
        current_dir = cd(clusters, path[num_dirs-1]);
        return current_dir;
    }
    if(strcmp(commands[0], "init") == 0){
        init(fat, boot_block, root_dir, clusters);
        return current_dir;
    }
    if(strcmp(commands[0], "load") == 0){
        load(boot_block, fat, root_dir, clusters);
        return current_dir;
    }
    if(strcmp(commands[0], "ls") == 0){
        if(commands[1] != NULL){
            if(commands[1][0] != '/') return current_dir;
            int num_dirs = get_num_lines(commands[1], '/') - 1;
            char **path = processar_string(commands[1], "/", num_dirs);
            current_dir = cd(clusters, path[num_dirs-1]);
        }
        ls(clusters, current_dir);
        return current_dir;
    }
    if(strcmp(commands[0], "mkdir") == 0){
        if(commands[1][0] != '/') return current_dir;
        int num_dirs = get_num_lines(commands[1], '/') - 1;
        char **path = processar_string(commands[1], "/", num_dirs);
        if(num_dirs > 1)current_dir = cd(clusters, path[num_dirs-2]);
        mkdir(fat, path[num_dirs-1], clusters, current_dir);
        save(fat, boot_block, root_dir, clusters);
        return current_dir;
    }
    if(strcmp(commands[0], "create") == 0){
        if(commands[1][0] != '/') return current_dir;
        int num_dirs = get_num_lines(commands[1], '/') - 1;
        char **path = processar_string(commands[1], "/", num_dirs);
        if(num_dirs > 1)current_dir = cd(clusters, path[num_dirs-2]);
        create(fat, path[num_dirs-1], clusters, current_dir);
        save(fat, boot_block, root_dir, clusters);
        return current_dir;
    }
    if(strcmp(commands[0], "unlink") == 0){
        if(commands[1][0] != '/') return current_dir;
        int num_dirs = get_num_lines(commands[1], '/' - 1);
        char **path = processar_string(commands[1], "/", num_dirs);
        if(num_dirs > 1)current_dir = cd(clusters, path[num_dirs-2]);
        unlink(fat, path[num_dirs-1], clusters, current_dir);
        save(fat, boot_block, root_dir, clusters);
        return current_dir;
    }
    if(strcmp(commands[0], "write") == 0){
        char text[256];
        for(int i = 1; i < num_linhas-1; i++){
            commands[i][strcspn(commands[i], " ")] = 0; 
            strcat(text, commands[i]);
            if(i < num_linhas-1) strcat(text, " ");
        }
        if(commands[num_linhas-1][0] != '/') return current_dir;
        int size = strlen(text);
        int num_dirs = get_num_lines(commands[num_linhas-1], '/') - 1;
        char **path = processar_string(commands[num_linhas-1], "/", num_dirs);
        if(num_dirs > 1)current_dir = cd(clusters, path[num_dirs-2]);
        write(fat,  path[num_dirs-1], text, size, clusters, current_dir);
        save(fat, boot_block, root_dir, clusters);
        return current_dir;
    }
    if(strcmp(commands[0], "append") == 0){
        char text[256];
        for(int i = 1; i < num_linhas-1; i++){
            commands[i][strcspn(commands[i], " ")] = 0; 
            strcat(text, commands[i]);
            if(i < num_linhas-1) strcat(text, " ");
        }
        if(commands[num_linhas-1][0] != '/') return current_dir;
        int size = strlen(text);
        int num_dirs = get_num_lines(commands[num_linhas-1], '/') - 1;
        char **path = processar_string(commands[num_linhas-1], "/", num_dirs);
        if(num_dirs > 1)current_dir = cd(clusters, path[num_dirs-2]);
        append(fat,  path[num_dirs-1], text, size, clusters, current_dir);
        save(fat, boot_block, root_dir, clusters);
        return current_dir;
    }
    if(strcmp(commands[0], "read") == 0){
        if(commands[1][0] != '/') return current_dir;
        int num_dirs = get_num_lines(commands[1], '/') - 1;
        char **path = processar_string(commands[1], "/", num_dirs);
        if(num_dirs > 1)current_dir = cd(clusters, path[num_dirs-2]);
        read(fat, path[num_dirs-1], clusters, current_dir);
        return current_dir;
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
    int current_dir = 0;
    do{ 
        printf("%s ", prompt);
        if(fgets(text, 100, stdin) == NULL) break;
        text[strcspn(text, "\n")] = 0;
        current_dir = get_command(text, fat, boot_block, root_dir, clusters, current_dir);
        if(strcmp(text, "fim") == 0) break;
    }while(strcmp(text, "fim\n") != 0 || (text[0] = getchar()) == EOF);
    return 0;
}