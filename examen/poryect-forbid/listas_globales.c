#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


char *forbiden_cmds[] = {NULL};




int main() {

    char cmd1[] = "ls";

    int num=0;

    forbiden_cmds[num] = NULL;
    
    forbiden_cmds[num]=malloc(strlen(cmd1) * sizeof(char)+1);
    
    strcpy(forbiden_cmds[num], cmd1);

    printf("%s \n", forbiden_cmds[num]);


    
}