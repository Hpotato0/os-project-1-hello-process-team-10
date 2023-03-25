# Project 1: hellon, process!
* Team 10: 김현석, 홍주원, 주재형
* test program directory: ./test/test_ptree.c

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
* inlcude/linux/syscall.h: system call의 proto type을 추가
* includ/uapi/asm-generic/uinstd.h: systemcall table에 등록한다. 294번 system call을 사용
* kernel/sys_ni.c: not implemented system을 위하여 함수를 등록한다.

## 3. Implement of ptree
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
다음과 같은 방식으로 test program을 작동시킨다.

## 5. Result of the test
```bash
root:~> ./projecet1_test 30
START TEST BINARY
#######################################################
ptree syscall returned: 30, errno: 0

swapper/0, 0, 0, 0
	systemd, 1, 1, 0
		dbus-daemon, 162, 1, 81
		systemd-journal, 164, 1, 0
		systemd-udevd, 189, 1, 0
		actd, 237, 1, 0
		buxton2d, 239, 1, 375
		key-manager, 240, 1, 444
		dlog_logger, 241, 1, 1901
		amd, 242, 1, 301
		alarm-server, 245, 1, 301
		bt-service, 246, 1, 551
		cynara, 248, 1, 401
		deviced, 250, 1, 0
		esd, 254, 1, 301
		license-manager, 255, 1, 402
		resourced-headl, 269, 1, 0
		login, 273, 1, 0
			bash, 648, 1, 0
				project1_test, 994, 0, 0
		net-config, 278, 1, 551
		murphyd, 301, 1, 451
		wpa_supplicant, 313, 1, 551
		connmand, 320, 1, 551
		update-manager, 326, 1, 202
		systemd-logind, 329, 1, 0
		mm-resource-man, 330, 1, 451
		bt-core, 354, 1, 551
		pulseaudio, 287, 1, 122
		focus_server, 373, 1, 451
#######################################################
END TEST BINARY
```
