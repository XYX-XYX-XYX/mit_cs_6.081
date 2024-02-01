#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int 
main(int argc,char *argv[])
{
	int n; 
	if(argc==1) {
        	write(1,"error\n",6);
		exit(0);
	}else{
		n = atoi(argv[1]);
		sleep(n);
		exit(0);
	}
}
