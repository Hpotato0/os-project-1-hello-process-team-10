# Project 1: hellon, process!
* Team 10: 김현석, 홍주원, 주재형
* test program directory: ~/project1_test

## 1. Background
## struct task_struct
project 1에서는 root process에서 부터 process tree를 순회하면서, process들의 state, pid, uid, name, depth를 저장하는 system call과 그리고 이 system call 함수를 호출하여 결과를 출력하는 test program을 짜는 것을 목표로 한다.
![image](https://user-images.githubusercontent.com/91672190/227708743-753bacc5-dabf-4047-86be-2ae908b86e2c.png)
이를 위하여 process에 대한 정보를 저장하는 task_struct의 정보들을 다루어야 한다. task_struct의 여러 member 중 우리가 활용해야 할 변수들은 다음과 같다.
* state: 현재 process의 state를 저장한다.
* cred: task의 credentiald을 저장하는 field로 task_uid macro를 통하여 uid를 구할 수 있다.
* pid: process의 pid를 저장
* comm: task의 executable name을 저장하는 변수
* sibling: list_head 구조체로 같은 parent를 가지고 있는 sibling을 linked list로 연결한다. siblint의 link 관계는 위의 그림과 같다.
* children: list_head 구조체로 해당 process의 children process의 sibling을 가리킨다.

## 2. Register system call
system call을 구현하기에 앞서 먼저 system call을 system call table에 추가해야 한다. 이를 위해서 다음 kernel code들을 수정해야 한다.
* inlcude/linux/syscall.h: system call의 proto typedmf 추가
* includ/uapi/asm-generic/uinstd.h: systemcall table에 등록한다. 294번 system call을 사용
* kernel/sys_ni.c: not implemented system을 위하여 함수를 등록한다.

## 3. Implementaion of ptree
sys_ptree의 함수는 kernel/ptree.c에 구현되어 있다. 현재 구현한 sys_ptree는 원형은 다음과 같다.
``` c
SYSCALL_DEFINE2(ptree, struct pinfo __user *, buf, size_t, len)
```
process의 정보를 저장할 user-land pointer buf 그리고 몇 개의 process를 순회해야하는 지에 대한 len을 input으로 받는다.
* error check logic
``` c
 43     // error detection
 44     if(buf == NULL || len == 0)
 45         return -EINVAL;
 46     if(!access_ok(VERIFY_WRITE, buf, len))
 47         return -EFAULT;
```
user space pointer가 유효하지 않은 영역일 경우 EFAULT 또는 pointer가 null이거나 len이 0일 경우 EINVAL을 추력한다.
* buffer alloc
```c
 49     // allocate
 50     alloc_unit = 64;
 51     sz = sizeof(struct pinfo) * (len > alloc_unit ? alloc_unit : len);
 52     pBuff = kzalloc(sz, GFP_KERNEL);
 53     if(!pBuff) return -ENOMEM;
```
kernel의 memory size는 무한하지 않으므로, 너무 큰 len을 입력으로 받았을 경우 문제가 발생할 수 있다. 이를 위하여 node의 정보를 저장하기 위한 buffer를 64개 이상 할당되지 않도록 한다. 대신 64개의 node를 접근하고 나서 user space에 process 정보를 복사한 후 다시 나머지 node들에 대한 순회 및 저장, user space로의 복사를 반복한다.
* pre-order tree traverse   
![image](https://user-images.githubusercontent.com/91672190/227717188-a43e45af-0310-4b62-890b-84fd15c67f06.png)
process tree를 pre-order로 순회하기 위해서 2개의 변수를 사용한다. Tree를 순회하던 중 중복되는 node를 저장하지 않기 위하여, 2개의 변수를 활용해 어떤 방향에서 node를 접근한 것인지 구분한다.   
부모 node로부터 접근했거나, sibling에서 접근했을 경우에만 node를 저장한다.

## 4. project1_test.c
test program은 argument로 ptree의 lenth를 argument로 받아 실행된다. 이 때 입력 argument는 1 이상의 정수여야 한다. 내부적으로 argument가 정수가 아니거나, 0 이하일 경우 system call을 호출하지 않고 종료된다.
그 후 내부에서 struct pinfo array를 length만큼 할당하고 system call을 호출하여 ptree의 정보를 system call 294번 즉 sys_ptree로 부터 ptree의 정보를 커널에서 받는다. 
system call의 결과가 음수가 아닐 경우 에러가 발생하지 않은 것으로 처리되며 standard out에 ptree를 출력하고 종료된다.

```bash
root:~> ./projecet1_test 50
```
다음과 같은 방식으로 test program을 작성한다.

## 5. Result of the test
```bash
root:~> ./projecet1_test 80
START TEST BINARY

number of pinfo: 80
swapper/0, 0, 0, 0
        systemd, 1, 1, 0
                dbus-daemon, 158, 1, 81
                systemd-journal, 163, 1, 0
                systemd-udevd, 189, 1, 0
                actd, 237, 1, 0
                buxton2d, 238, 1, 375
                key-manager, 239, 1, 444
                dlog_logger, 242, 1, 1901
                amd, 244, 1, 301
                alarm-server, 245, 1, 301
                bt-service, 246, 1, 551
                cynara, 247, 1, 401
                deviced, 249, 1, 0
                esd, 255, 1, 301
                license-manager, 257, 1, 402
                resourced-headl, 263, 1, 0
                login, 271, 1, 0
                        bash, 1050, 1, 0
                                projecet1_test, 1240, 0, 0
                net-config, 279, 1, 551
                murphyd, 285, 1, 451
                connmand, 302, 1, 551
                systemd-logind, 318, 1, 0
                wpa_supplicant, 327, 1, 551
                update-manager, 337, 1, 202
                mm-resource-man, 322, 1, 451
                pulseaudio, 283, 1, 122
                bt-core, 353, 1, 551
                focus_server, 356, 1, 451
                tlm, 366, 1, 0
                        tlm-sessiond, 390, 1, 0
                                bash, 438, 1, 5001
                muse-server, 384, 1, 451
                bluetoothd, 400, 1, 551
                systemd, 434, 1, 5001
                        (sd-pam), 435, 1, 5001
                        sh, 443, 1, 5001
                                sleep, 1239, 1, 5001
                        launchpad-proce, 480, 1, 5001
                                update-agent, 571, 0, 5001
                                smartthings-thi, 593, 1, 5001
                        dbus-daemon, 595, 1, 5001
                pass, 499, 1, 202
                storaged, 501, 1, 0
                security-manage, 574, 1, 0
                        security-manage, 577, 1, 0
        kthreadd, 2, 1, 0
                rcu_gp, 3, 1026, 0
                rcu_par_gp, 4, 1026, 0
                kworker/0:0, 5, 1026, 0
                kworker/0:0H, 6, 1026, 0
                kworker/u8:0, 7, 1026, 0
                mm_percpu_wq, 8, 1026, 0
                ksoftirqd/0, 9, 1, 0
                rcu_preempt, 10, 1026, 0
                rcu_sched, 11, 1026, 0
                rcu_bh, 12, 1026, 0
                migration/0, 13, 1, 0
                cpuhp/0, 14, 1, 0
                cpuhp/1, 15, 1, 0
                migration/1, 16, 1, 0
                ksoftirqd/1, 17, 1, 0
                kworker/1:0, 18, 1026, 0
                kworker/1:0H, 19, 1026, 0
                cpuhp/2, 20, 1, 0
                migration/2, 21, 1, 0
                ksoftirqd/2, 22, 1, 0
                kworker/2:0, 23, 1026, 0
                kworker/2:0H, 24, 1026, 0
                cpuhp/3, 25, 1, 0
                migration/3, 26, 1, 0
                ksoftirqd/3, 27, 1, 0
                kworker/3:0, 28, 1026, 0
                kworker/3:0H, 29, 1026, 0
                kdevtmpfs, 30, 1, 0
                netns, 31, 1026, 0
                rcu_tasks_kthre, 32, 1, 0
                kauditd, 33, 1, 0
                kworker/0:1, 34, 1026, 0

END TEST BINARY
```
