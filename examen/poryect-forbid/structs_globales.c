#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>





struct forbidden_cmds {

    int numCmd;
    char **cmds;

};


typedef struct forbidden_cmds forbidden_cmds;


void
initializer_cmds(forbidden_cmds *forbid) {

    forbid->numCmd=0;
    forbid->cmds=NULL;

}

void
reserve_cmds(forbidden_cmds *forbid, char *cmd1) {
    forbid->cmds=realloc(forbid->cmds, (forbid->numCmd + 1) * sizeof(char *));
    forbid->cmds[forbid->numCmd]=malloc(strlen(cmd1)* sizeof(char) +1);

    forbid->cmds[forbid->numCmd][0] = '\0';
    strcpy(forbid->cmds[forbid->numCmd], cmd1);
    forbid->numCmd = forbid->numCmd +1;
}


int
main(int argc, char *argv[]) {

    char *cmd1 = "ls";
    forbidden_cmds forbid;

    initializer_cmds(&forbid);

    reserve_cmds(&forbid, cmd1);

   

    for (int numCommand = 0; numCommand < forbid.numCmd; numCommand++) {
         printf("cmd: %s. \n", forbid.cmds[numCommand]);
    }
    
    for (int numCommand = 0; numCommand < forbid.numCmd; numCommand++) {
        free(forbid.cmds[numCommand]);
    }

    free(forbid.cmds);
}