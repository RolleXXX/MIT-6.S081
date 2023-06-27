#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "sysinfo.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

<<<<<<< HEAD
uint64 
sys_trace(void) {
  int trace_sys_mask;
  if (argint(0, &trace_sys_mask) < 0)
    return -1;
  myproc()->tracemask |= trace_sys_mask;
  return 0;
}

//不知道为啥我找不到这几个函数
extern uint64 kfreemem(void);
extern uint64 count_free_proc(void);

uint64
sys_sysinfo(void){
  struct proc *my_proc = myproc();
  uint64 p;
  // 获取用户提供的buffer地址
  if(argaddr(0,&p)<0){
    return -1;
  }

  struct sysinfo s;
  s.freemem = kfreemem();
  s.nproc = count_free_proc();

  if(copyout(my_proc->pagetable,p,(char*)&s,sizeof(s))<0){
    return -1;
  }
=======
uint64
sys_trace(void)
{
  // 拿到系统调用的数字
  int mask;
  if(argint(0,&mask)<0)
    return -1;
  // 做相与 实际上是把要追踪的那一位设为1
  printf("mask: %d\n", mask);
  myproc()->tracemask |= mask;
>>>>>>> util
  return 0;
}
