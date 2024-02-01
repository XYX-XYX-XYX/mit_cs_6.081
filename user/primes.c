#include "kernel/types.h"
#include "user/user.h"

void prime(int* p)
{
    int pc[2];
    int div;
    int num;

    close(p[1]);
    if(read(p[0],&div,4) == 0) return;
    printf("prime %d\n",div);
    pipe(pc);
    if(fork() == 0){
        prime(pc);
        exit(0);
    }else{
        close(pc[0]);
        while(read(p[0],&num,4) != 0){
            if(num % div != 0){
                write(pc[1],&num,4);
            }
        }
       
        close(p[0]);
        close(pc[1]);
        wait(0);
        exit(0);
    }
}
int
main(int argc,char** argv)
{
    int i;
    int p[2];
    pipe(p);
    if(fork() == 0){
        prime(p);
        exit(0);
    }else{
        close(p[0]);
        for(i = 2;i <= 35;i++){
            write(p[1],&i,4);
        }
        close(p[1]);
        wait(0);
    }
    exit(0);
}