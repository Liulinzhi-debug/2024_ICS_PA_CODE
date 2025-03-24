#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
  
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}


#define VA_OFFSET(addr) (addr & 0xFFF) 
#define VA_VPN_0(addr)  ((addr >> 12) & 0x3FF) 
#define VA_VPN_1(addr)  ((addr >> 22) & 0x3FF) 

#define PA_OFFSET(addr) (addr & 0xFFF) 
#define PA_PPN(addr)    ((addr >> 12) & 0xFFFFF) 
#define PTE_PPN 0xFFFFF000 

#define GET_PAGE_TABLE_BASE(pte) ((PTE *) ((uintptr_t) (pte) & PTE_PPN))


void map(AddrSpace *as, void *va, void *pa, int prot) {
    uintptr_t va_trans = (uintptr_t)va;
    uintptr_t pa_trans = (uintptr_t)pa;
    assert(PA_OFFSET(pa_trans) == 0);
    assert(VA_OFFSET(va_trans) == 0);

    uint32_t ppn = PA_PPN(pa_trans);
    uint32_t vpn_1 = VA_VPN_1(va_trans);
    uint32_t vpn_0 = VA_VPN_0(va_trans);
    //printf("map里va是%x pa是%x\n",va,pa);
    PTE *page_dir = (PTE *)as->ptr;
    //printf("页目录的基地址as->ptr = 0x%x\n",(PTE *)as->ptr);
    PTE *pde = &page_dir[vpn_1];

    if ((*pde & PTE_V) == 0) {
        void *pt_phys = pgalloc_usr(PGSIZE);
        memset(pt_phys, 0, PGSIZE);
        
        *pde = (PTE)pt_phys | PTE_V;
        
        // printf(
        //     "分配二级页表: pde地址=0x%x, 值=0x%x (物理页表基址=0x%x)\n",
        //     (uintptr_t)pde, *pde, (uintptr_t)pt_phys
        // );
    }

    PTE *ptab = (PTE *)(*pde & PTE_PPN);
    PTE *pte = &ptab[vpn_0];
    
    *pte = (ppn << 12) | PTE_V | PTE_R|PTE_W|PTE_X;
    // printf(
    //     "设置二级页表项: pte地址=0x%x, 值=0x%x (物理页=0x%x)\n",
    //     (uintptr_t)pte, *pte, pa_trans
    // )
    ;
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  //return NULL;

  Context* context = kstack.end - sizeof(Context);
  memset(context, 0, sizeof(Context));
  context->pdir = as->ptr;
  context->mepc = (uintptr_t)entry;

 context->mstatus= 0x80 ;
  //context->mstatus= 0x1800;
  context->gpr[0] = 0;
  context->mcause = 0;
  context->np= (uintptr_t)kstack.end;
  return context;
}
