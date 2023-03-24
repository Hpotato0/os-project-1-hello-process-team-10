#include <linux/syscalls.h>
#include <linux/pinfo.h> // struct pinfo
#include <linux/sched/task.h> // locks
#include <linux/slab.h> // kzalloc
#include <linux/gfp.h> // GFP_KERNEL
#include <uapi/asm-generic/errno-base.h> // error codes

SYSCALL_DEFINE2(ptree, struct pinfo *, buf, size_t, len)
{
    size_t alloc_unit;  // allocation unit
    size_t sz;          // size to allocate
    struct pinfo* pBuff;// allocated buffer pointer
    size_t pcount;      // pinfo read count

    // for traversing tree
    struct task_struct* taskptr;
    int going_down;
    int cur_depth;

    // debug msg
    printk("running syscall ptree\n");

    // error detection
    if(buf == NULL || len == 0)
        return -EINVAL;

    // allocate
    alloc_unit = 64;
    sz = sizeof(struct pinfo) * (len > alloc_unit ? alloc_unit : len);
    pBuff = kzalloc(sz, GFP_KERNEL);
    if(!pBuff) return -ENOMEM;
    
    taskptr = &init_task;
    pcount = 0;
    going_down = 1;
    while(pcount < len){
        size_t end;
        if(pcount + alloc_unit > len) end = len - pcount;
        else end = alloc_unit;
        pcount += alloc_unit;

        read_lock(&tasklist_lock);

        for(size_t idx = 0; idx < end; idx++){
            SAVE taskptr to pBuff[idx]
            IF taskptr has child:
                going_down
        }

        read_unlock(&tasklist_lock);
    }

    return 0;
}