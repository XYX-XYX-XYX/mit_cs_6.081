#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

int
main(int argc , char** argv)
{
    char buf[MAXARG][10];
    char* argm[MAXARG];
    char c[1];
    int i,ii;
    int k = 0;

    for(i = 1;i < argc;i++){
        strcpy(buf[i-1],argv[i]);
        argm[i-1] = buf[i-1];
    }
    i--;
    ii = i;
    
    while( read(0,c,1) != 0 ){
        if( *c == ' ' ){
            buf[i][k] = '\0';
            argm[i] = buf[i];
            k = 0;
            i++;
        }else if( *c == '\n' ){
            buf[i][k] = '\0';
            argm[i] = buf[i];
            i++;
            k=0;
            argm[i] = 0;
            i=ii;
            if(fork() == 0){
                exec(argm[0],argm);
            }else{
                wait(0);
            }
        }else{
            buf[i][k] = *c;
            k++;
        }
    }
    exit(0);
}