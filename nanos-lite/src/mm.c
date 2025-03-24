#include <memory.h>
#include <proc.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
  //return NULL;
  void *last_pf = pf;
  pf += nr_page * PGSIZE;
   //printf("new_page中pf是%x,last_pf是%x\n",pf,last_pf);
 // assert(pf < (void *)heap.end);
  return last_pf;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  assert(n % PGSIZE==0);
  void *p = new_page(n / PGSIZE);

  if (p == NULL) panic("Out of physical memory");
   memset(p, 0, n);
  return p;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
// int mm_brk(uintptr_t brk) {
//   return 0;
// }
int mm_brk(uintptr_t brk) {

current->max_brk = ROUNDUP(current->max_brk, PGSIZE);
  if (brk > current->max_brk) {
    int page_count = ROUNDUP(brk - current->max_brk, PGSIZE) >> 12;
    uintptr_t pages_start = (uintptr_t)new_page(page_count);
    for (int i = 0; i < page_count; ++ i) {
      map(&current->as, 
          (void*)(current->max_brk + i * PGSIZE), 
          (void*)(pages_start + i * PGSIZE),
          0
          );
    }
    current->max_brk += page_count * PGSIZE;
  }

  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %x", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
