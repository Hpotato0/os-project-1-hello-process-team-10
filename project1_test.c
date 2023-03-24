#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <errno.h>
#include <string.h>
#include <stdlib.h>

struct pinfo {
  int64_t  state;       /* current state of the process */
  pid_t pid;         /* process id */
  int64_t  uid;         /* user id of the process owner */
  char          comm[64];    /* name of the program executed */
  unsigned int  depth;       /* depth of the process in the process tree */
};

int main(int argc, char* argv[]){
    long ret;
    int errnum;
    struct pinfo *data;
    int length;
    int i, j;
    struct pinfo p;
    printf("START TEST BINARY\n\n");

    if(!isdigit((*argv[1])))
    {
        printf("Length should be a integer");
        return 0;
    }
    else if(atoi(argv[1]) <= 0)
    {
        printf("Length should be bigger than 0");
        return 0;
    }
    length = atoi(argv[1]);
    data = (struct pinfo*)malloc(length*sizeof(struct pinfo));
    ret = syscall(294, data, length);
    errnum = errno;

    if(ret < 0)
    {
        printf("Error during system call: %s\n\n", strerror(errnum));
    }
    else
    {
        printf("number of pinfo: %d\n", ret);
        for(i=0;i<ret+5;i++)
        {
            p = data[i];
            for(j=0;j<p.depth;j++)
                printf("\t");
            printf("%s, %d, %lld, %lld\n", p.comm, p.pid, p.state, p.uid);
        }
    }

    printf("\nEND TEST BINARY\n");
}

// aarch64-linux-gnu-gcc test_binary.c -o test_binary -static