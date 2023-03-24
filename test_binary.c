#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <errno.h>
#include <string.h>

struct pinfo {
  int64_t  state;       /* current state of the process */
  pid_t pid;         /* process id */
  int64_t  uid;         /* user id of the process owner */
  char          comm[64];    /* name of the program executed */
  unsigned int  depth;       /* depth of the process in the process tree */
};

int main(void){
    printf("START TEST BINARY\n\n");
    
    long ret; int errnum;
    struct pinfo data[10];

    ret = syscall(294, NULL, 10); errnum = errno;
    printf("syscall returned: %ld\t", ret); printf("Value of errno: %d\n", errnum);
    if(ret < 0) fprintf(stderr, "Error opening file: %s\n\n", strerror(errnum));

    ret = syscall(294, data, 0); errnum = errno;
    printf("syscall returned: %ld\t", ret); printf("Value of errno: %d\n", errnum);
    if(ret < 0) fprintf(stderr, "Error opening file: %s\n\n", strerror(errnum));

    ret = syscall(294, data, 10); errnum = errno;
    printf("syscall returned: %ld\t", ret); printf("Value of errno: %d\n", errnum);
    if(ret < 0) fprintf(stderr, "Error opening file: %s\n\n", strerror(errnum));

    printf("\nEND TEST BINARY\n");
}

// aarch64-linux-gnu-gcc test_binary.c -o test_binary -static