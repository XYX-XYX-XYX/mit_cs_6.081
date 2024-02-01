#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char * 
findName(char *path)
{
    char* p;
    for(p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;
  return p;
}

void 
findRecursely(char *path,char *file)
{
    char buff[512] = {0};
    char* p;
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(path, O_RDONLY)) < 0){
      fprintf(2, "find: cannot open %s\n", path);
      return;
    }

    if(fstat(fd, &st) < 0){
      fprintf(2, "find: cannot stat %s\n", path);
      close(fd);
      return;
    }

    switch (st.type)
    {
    case T_DEVICE:
        break;
    case T_FILE:
        if(strcmp( findName(path),file ) == 0){
            printf("%s\n",path);
        }
        break;
    default:
        strcpy(buff, path);
        p = buff+strlen(buff);
        *p++ = '/';
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0)
                continue;
            memmove(p,de.name,DIRSIZ);
            p[DIRSIZ] = 0;
            if( (strcmp(findName(buff),".") != 0) &&(strcmp(findName(buff),"..") != 0) ){
                findRecursely(buff,file);
            }
        }
        break;
    }
    close(fd);
}

int 
main(int argc ,char** argv)
{
    if(argc < 3)
    {
        printf("error\n");
        exit(1);
    }
    char* path = argv[1];
    char* file = argv[2];

    findRecursely(path,file);
    exit(0);
}