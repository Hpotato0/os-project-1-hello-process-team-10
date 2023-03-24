#include <linux/syscalls.h>
#include <linux/pinfo.h> // struct pinfo
#include <linux/sched/task.h> // locks
#include <linux/slab.h> // kzalloc
#include <linux/gfp.h> // GFP_KERNEL
#include <uapi/asm-generic/errno-base.h> // error codes
#include <linux/cred.h> // cred->kuid_t
#include <linux/sched.h> //TASK_COMM_LEN

static void savePinfoToBuff(struct task_struct* taskptr, struct pinfo* pBuff, unsigned int cur_depth){
    pBuff->state = taskptr->state;
    pBuff->pid = taskptr->pid;
    pBuff->uid = (taskptr)->cred->uid.val;

    __get_task_comm(pBuff->comm, TASK_COMM_LEN, taskptr);
    pBuff->depth = cur_depth;
}

SYSCALL_DEFINE2(ptree, struct pinfo *, buf, size_t, len)
{
    size_t alloc_unit;  // allocation unit
    size_t sz;          // size to allocate
    struct pinfo* pBuff;// allocated buffer pointer
    size_t pcount;      // pinfo read count
    unsigned long failed_copy_bytes;// for checking copy_to_user failure

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
        size_t idx;
        size_t sz_copy;

        start = pcount;
        if(pcount + alloc_unit > len) end = len;
        else end = pcount + alloc_unit;

        idx = 0;
        read_lock(&tasklist_lock);
        while(pcount < end){
            //control: going_down, from_child
            //counter: depth, copied_num
            //depth is changed when jump to next node
            //copied_num is changed when taskptr is visited
            //goint_down & from_child is changed when jump to next node
            if(cur_depth == 0 && going_down==0) break;

            if(going_down){ // must visit taskptr
                savePinfoToBuff(taskptr, pBuff+idx, cur_depth);                
                idx++; pcount++;

                if(!list_empty(&(taskptr->children))){
                    taskptr = list_first_entry(&(taskptr->children), struct task_struct, children);
                    cur_depth++;
                }
                else{
                    going_down = 0;
                    if(!list_empty(&(taskptr->sibling))){
                        taskptr = list_first_entry(&(taskptr->sibling), struct task_struct, sibling);
                    }
                    else{
                        taskptr = taskptr->real_parent;
                        from_child = 1;
                        cur_depth--;
                    }
                }
            }
            else{
                if(from_child){ // last node was its child
                    if(!list_empty(&(taskptr->sibling))){
                        from_child = 0;
                        taskptr = list_first_entry(&(taskptr->sibling), struct task_struct, sibling);
                    }
                    else{ // taskptr doesn't have next sibling => go to parent
                        taskptr = taskptr->real_parent;
                        from_child = 1;
                        cur_depth--;
                    }
                }
                else{// last node was its sibling, must visit taskptr
                    savePinfoToBuff(taskptr, pBuff+idx, cur_depth);                
                    idx++; pcount++;

                    if(!list_empty(&(taskptr->children))){
                        going_down = 1;
                        taskptr = list_first_entry(&(taskptr->children), struct task_struct, children);
                        cur_depth++;
                    }
                    else{
                        if(!list_empty(&(taskptr->sibling))){
                            taskptr = list_first_entry(&(taskptr->sibling), struct task_struct, sibling);
                        }
                        else{
                            taskptr = taskptr->real_parent;
                            from_child = 1;
                            cur_depth--;
                        }
                    }
                }
            }
        }
        read_unlock(&tasklist_lock);

        sz_copy = sizeof(struct pinfo) * idx;
        failed_copy_bytes = copy_to_user(buf, pBuff, sz_copy);
    }

    return pcount;
}