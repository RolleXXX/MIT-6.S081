// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

void
kinit()
{
  for (int i = 0; i < NCPU;i++){
    char name[9] = {0};
    snprintf(name, 8, "kmem-%d", i);
    initlock(&kmem[i].lock, "kmem");
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // 先关闭中断，防止CPU1的代码放到CPU2上运行
  push_off();
  int cpu = cpuid();

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem[cpu].lock);
  r->next = kmem[cpu].freelist;
  kmem[cpu].freelist = r;
  release(&kmem[cpu].lock);

  pop_off();// 打开中断
}

void* ksteal(int cpu){
  struct run *r;
  for (int i = 1; i < NCPU;i++){
    int next_cpu = (cpu + i) % NCPU;

    acquire(&kmem[next_cpu].lock);
    r = kmem[next_cpu].freelist;
    if(r){
      kmem[next_cpu].freelist = r->next;
    }
    release(&kmem[next_cpu].lock);
    if(r){
      break;
    }
  }
  return r;
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  push_off();

  int cpu = cpuid();

  acquire(&kmem[cpu].lock);
  r = kmem[cpu].freelist;
  if (r)
    kmem[cpu].freelist = r->next;
  release(&kmem[cpu].lock);

  if(r==0){
    // 从别的cpu上偷
    r = ksteal(cpu);
  }

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  
  pop_off();
  return (void*)r;
}
