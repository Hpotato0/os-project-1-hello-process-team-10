#include <linux/syscalls.h>
#include <linux/pinfo.h> // struct pinfo
#include <linux/sched/task.h> // locks
#include <linux/slab.h> // kzalloc
#include <linux/gfp.h> // GFP_KERNEL
#include <uapi/asm-generic/errno-base.h> // error codes

static void savePinfoToBuff(struct task_struct* taskptr, struct pinfo* pBuff, unsigned int cur_depth){
    pBuff->state = taskptr->state;
    pBuff->pid = taskptr->pid;
    pBuff->uid = (taskptr->cred)->uid;
    get_task_comm(pBuff->comm, taskptr);
    pBuff->depth = cur_depth;
}

SYSCALL_DEFINE2(ptree, struct pinfo *, buf, size_t, len)
{
    size_t alloc_unit;  // allocation unit
    size_t sz;          // size to allocate
    struct pinfo* pBuff;// allocated buffer pointer
    size_t pcount;      // pinfo read count

    // for traversing tree
    struct task_struct* taskptr;
    int going_down = 1;
    int from_child = 0;
    unsigned int cur_depth = 0;

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
        size_t start;
        size_t end;

        start = pcount;
        if(pcount + alloc_unit > len) end = len;
        else end = pcount + alloc_unit;

        read_lock(&tasklist_lock);
        while(pcount < end){
            //control: going_down, from_child
            //counter: depth, copied_num
            //depth is changed when jump to next node
            //copied_num is changed when taskptr is visited
            //goint_down & from_child is changed when jump to next node
            size_t idx = 0;
            if(cur_depth == 0 && going_down==0) break;

            if(going_down){ // must visit taskptr
                savePinfoToBuff(taskptr, pBuff+idx, cur_depth);                
                idx++; pcount++;

                if(!list_empty(taskptr->children)){
                    taskptr = list
                }
                else{

                }

                IF taskptr has child: // go to child
                    taskptr = leftmost child of taskptr;
                    cur_depth++
                ELSE: // check if taskptr has sibling. If so, go to sibling, if not, go to parent
                    going_down = 0;
                    IF taskptr has next sibling:
                        taskptr = next sibling of taskptr;
                    ELSE:
                        taskptr = parent of taskptr;
                        cur_depth--;
                        from_child = 1;
            }
            else{
                IF from_child is 1: // last node was its child
                    IF taskptr has next sibling:
                        from_child = 0;
                        taskptr = next sibling of taskptr;
                    ELSE: // taskptr doesn't have next sibling => go to parent
                        taskptr = parent of taskptr;
                        from_child = 1;
                        cur_depth--;
                ELSE: // last node was its sibling, must visit taskptr
                    copied_num++;
                    SAVE taskptr and cur_depth to pBuff[idx];

                    IF tasktpr has child:
                        going_down = 1;
                        taskptr = child of taskptr;
                        cur_cepth++;
                    ELSE: // taskptr doesn't have child
                        IF takptr has next sibling:
                            tasktpr = next sibling of taskptr
                        ELSE: // taskptr doesn't have next sibling
                            from_child = 1;
                            cur_depth--;
                            taskptr = parent of taskptr;
            }         
        }

        read_unlock(&tasklist_lock);
    }

    return 0;
}