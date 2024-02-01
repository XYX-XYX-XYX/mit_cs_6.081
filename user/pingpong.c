#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc , char** argv){
    int p1[2],p2[2];
    char buff[1];
    int pid;
    buff[0] = '!';

    pipe(p1);
    pipe(p2);

    if( (pid = fork()) < 0 ){
        printf("error!");
        exit(1);
    }else if(pid != 0){
        close(p1[0]);
        write(p1[1],"p",1);
        close(p1[1]);
        close(p2[1]);
        read(p2[0],buff,1);
        close(p2[0]);
        printf("%d: received pong\n",getpid());
    }else{
        close(p1[1]);
        read(p1[0],buff,1);
        close(p1[0]);
        printf("%d: received ping\n",getpid());
        close(p2[0]);
        write(p2[1],"c",1);
        close(p2[1]);
        exit(0);
    }
    exit(0);
}