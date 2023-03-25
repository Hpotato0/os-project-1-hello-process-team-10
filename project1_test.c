#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h> //isdigit

struct pinfo {
  int64_t  state;       /* current state of the process */
  pid_t pid;         /* process id */
  int64_t  uid;         /* user id of the process owner */
  char          comm[64];    /* name of the program executed */
  unsigned int  depth;       /* depth of the process in the process tree */
};

// length_passed_to_ptree length_of_user_buffer
int main(int argc, char* argv[]){
    long ret;
    int errnum;
    struct pinfo *data;
    int length;
    int i, j;
    struct pinfo p;
    printf("START TEST BINARY\n#######################################################\n");

    if(!isdigit((*argv[1])))
    {
        printf("Length should be a integer");
        return 0;
    }

    int len_ptree = 0;
    int len_user = 0;
    
    if(argc == 2){
        len_ptree = atoi(argv[1]);
        len_user = len_ptree;
    }
    else{
        printf("invalid number of arguments\n");
        return -1;
    }


    data = (struct pinfo*)malloc(len_user*sizeof(struct pinfo));
    ret = syscall(294, data, len_ptree);
    errnum = errno;

    printf("ptree syscall returned: %ld, errno: %d\n", ret, errnum);

    if(ret < 0)
    {
        if(errnum == 22) printf("error EINVAL during system call\n");
        else if(errnum == 14) printf("error EFAULT during system call\n");
        else printf("misc error during system call: %s\n", strerror(errnum));
    }
    else
    {
        printf("\n");
        for(i=0;i<ret;i++)
        {
            p = data[i];
            for(j=0;j<p.depth;j++)
                printf("\t");
            printf("%s, %d, %ld, %ld\n", p.comm, p.pid, p.state, p.uid);
        }
    }

    printf("#######################################################\nEND TEST BINARY\n");

    free(data);
    return 0;
}

// aarch64-linux-gnu-gcc test_binary.c -o test_binary -static