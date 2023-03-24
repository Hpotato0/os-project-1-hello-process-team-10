#include <linux/syscalls.h>
#include <linux/pinfo.h> // struct pinfo
#include <linux/sched/task.h> // locks
#include <linux/slab.h> // kzalloc
#include <linux/gfp.h> // GFP_KERNEL
#include <uapi/asm-generic/errno-base.h> // error codes
#include <linux/cred.h> // cred->kuid_t
#include <linux/sched.h> //TASK_COMM_LEN

static void savePinfoToBuff(struct task_struct* taskptr, struct pinfo* pBuff, unsigned int cur_depth){
    
    printk("save 1\n");
    pBuff->state = taskptr->state;
    printk("save 2\n");
    pBuff->pid = taskptr->pid;
    printk("save 3\n");
    pBuff->uid = (task_uid(taskptr)).val;
    printk("save 4\n");
    __get_task_comm(pBuff->comm, TASK_COMM_LEN, taskptr);
    printk("save 5\n");
    pBuff->depth = cur_depth;
    printk("save 6\n");
}

SYSCALL_DEFINE2(ptree, struct pinfo __user *, buf, size_t, len)
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

        printk("pcount: %ld\n", pcount);

        start = pcount;
        if(pcount + alloc_unit > len) end = len;
        else end = pcount + alloc_unit;

        printk("end: %ld\n", end);

        idx = 0;
        read_lock(&tasklist_lock);
        while(pcount < end){
            //control: going_down, from_child
            //counter: depth, copied_num
            //depth is changed when jump to next node
            //copied_num is changed when taskptr is visited
            //goint_down & from_child is changed when jump to next node
            printk("pcount: %ld\n", pcount);
            printk("pointer value: %x\n", taskptr);
            printk("going_down =: %d\n", going_down);
            printk("from_child: %d\n", from_child);
            printk("depth: %d\n\n", cur_depth);

            if(cur_depth == 0 && going_down==0) break;

            if(going_down){ // must visit taskptr
                printk("@saving1\n");
                savePinfoToBuff(taskptr, pBuff+idx, cur_depth);
                printk("@save done1\n");           
                idx++; pcount++;

                if(!list_empty(&(taskptr->children))){
                    printk("cpath1 begins\n");
                    taskptr = list_first_entry(&(taskptr->children), struct task_struct, sibling);
                    cur_depth++;
                    printk("cpath1 ends\n");
                }
                else{
                    going_down = 0;
                    if((taskptr->sibling).next != NULL){
                        printk("cpath2-1 begins\n");
                        taskptr = list_first_entry(&(taskptr->sibling), struct task_struct, sibling);
                        printk("cpath2-1 ends\n");
                    }
                    else{
                        printk("cpath2-2 begins\n");
                        taskptr = taskptr->real_parent;
                        from_child = 1;
                        cur_depth--;
                        printk("cpath2-2 ends\n");
                    }
                }
            }
            else{
                if(from_child){ // last node was its child
                    if((taskptr->sibling).next != NULL){
                        printk("cpath3-1 begins\n");
                        from_child = 0;
                        taskptr = list_first_entry(&(taskptr->sibling), struct task_struct, sibling);
                        printk("cpath3-1 ends\n");
                    }
                    else{ // taskptr doesn't have next sibling => go to parent
                        printk("cpath3-2 begins\n");
                        taskptr = taskptr->real_parent;
                        from_child = 1;
                        cur_depth--;
                        printk("cpath3-2 ends\n");
                    }
                }
                else{// last node was its sibling, must visit taskptr
                    printk("@saving2\n");
                    savePinfoToBuff(taskptr, pBuff+idx, cur_depth);
                    printk("@saving done2\n");                
                    idx++; pcount++;

                    if(!list_empty(&(taskptr->children))){
                        printk("cpath4-1 begins\n");
                        going_down = 1;
                        taskptr = list_first_entry(&(taskptr->children), struct task_struct, sibling);
                        cur_depth++;
                        printk("cpath4-1 ends\n");
                    }
                    else{
                        if((taskptr->sibling).next != NULL){
                            printk("cpath4-2-1 begins\n");
                            taskptr = list_first_entry(&(taskptr->sibling), struct task_struct, sibling);
                            printk("cpath4-2-1 ends\n");
                        }
                        else{
                            printk("cpath4-2-2 begins\n");
                            taskptr = taskptr->real_parent;
                            from_child = 1;
                            cur_depth--;
                            printk("cpath4-2-2 ends\n");
                        }
                    }
                }
            }
        }
        read_unlock(&tasklist_lock);

        printk("final index: %ld\n", idx);

        sz_copy = sizeof(struct pinfo) * idx;
        failed_copy_bytes = copy_to_user(buf, pBuff, sz_copy);
    }

    return pcount;
}