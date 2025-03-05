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
} kmem;

struct //这个结构体是用来记录每个物理页面的引用数的
{
  struct spinlock lock;//物理地址最大是phystop
  int ref[PHYSTOP/PGSIZE];//这里是数组的最大值，不是说现在已经有这么多计数
}pageRef;


void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&pageRef.lock, "Pageref");
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

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");


  acquire(&pageRef.lock);//注意，共享计数只有在这里才会往下减，因为需要结合计数只剩1这个关键变量做判断，是否释放物理页。
  if(pageRef.ref[(uint64)pa / PGSIZE ] > 1 ){
    pageRef.ref[(uint64)pa / PGSIZE ] -= 1;
    release(&pageRef.lock);
    return;
  }
  release(&pageRef.lock);
  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    pageRef.ref[(uint64)r / PGSIZE] = 1;//分配页面的时候把计数设置为1
  }
    
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

int
intCount(uint64 pa){//这个函数用于添加计数
  if((pa % PGSIZE) != 0 || (char*)pa < end || pa >= PHYSTOP)
    return -1;
  acquire(&pageRef.lock);
  pageRef.ref[pa / PGSIZE] += 1;
  release(&pageRef.lock);
  return 1;
}

int
getRefNum(uint64 pa){//这个函数用于获取计数
  return pageRef.ref[pa / PGSIZE];
}