#include <proc.h>
#include <elf.h>
#include <klib.h>
#include <fs.h>

size_t ramdisk_read(void *buf, size_t offset, size_t len);
int fs_open(const char *pathname, int flags, int mode);
int fs_close(int fd);
size_t fs_lseek(int fd, size_t offset, int whence);
Context *ucontext(AddrSpace *as, Area kstack, void *entry);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_read(int fd, void *buf, size_t len);
void* new_page(size_t nr_page);
#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif


static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr ehdr;

  int fd = fs_open(filename, 0, 0); 
  assert(fd != -1);

  fs_read(fd, &ehdr, sizeof(ehdr));

  // char riscv32_magic_num[] = {0x7f, 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  // assert(strcmp((char *)(ehdr.e_ident), riscv32_magic_num) == 0);

 // Elf_Phdr phdr[ehdr.e_phnum];
 // fs_lseek(fd, ehdr.e_phoff, SEEK_SET);
 // fs_read(fd, phdr, sizeof(Elf_Phdr) * ehdr.e_phnum);

  for (int i = 0; i < ehdr.e_phnum; i++) {

     Elf_Phdr phdr;
    fs_lseek(fd, ehdr.e_phoff + i *ehdr.e_phentsize, SEEK_SET);
    fs_read(fd, &phdr, sizeof(Elf_Phdr));
    if (phdr.p_type == PT_LOAD) {

      uintptr_t vpage_start = phdr.p_vaddr & (~0xfff); 
      uintptr_t vpage_end = (phdr.p_vaddr + phdr.p_memsz - 1) & (~0xfff); 
      int page_num = ((vpage_end - vpage_start) >> 12) + 1; 

 
      uintptr_t page_ptr = (uintptr_t)new_page(page_num);
       printf("page_ptr是%x\n",page_ptr);

      for (int j = 0; j < page_num; ++j) {
        map(&pcb->as, 
            (void*)(vpage_start + (j << 12)), 
            (void*)(page_ptr + (j << 12)), 
            0);
      }
        
      void* page_off = (void *)(phdr.p_vaddr & 0xfff);
      memset((page_ptr + page_off+ phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
      fs_lseek(fd, phdr.p_offset, SEEK_SET);
      //fs_read(fd, (void *)phdr.p_vaddr, phdr.p_filesz);
      printf("loader里page_ptr + page_off是%x\n",page_ptr + page_off);
      fs_read(fd, page_ptr + page_off,phdr.p_filesz); 
      
      pcb->max_brk = vpage_end + PGSIZE;
       printf("loader里i是%x,程序区间是%x-%x\n",i,phdr.p_vaddr,phdr.p_vaddr+phdr.p_memsz);
      //  fs_lseek(fd, phdr.p_offset, SEEK_SET);
      //  fs_read(fd, (void *)phdr.p_vaddr, phdr.p_filesz);
      //  memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);

     }
  }
  assert(fs_close(fd) != -1);
  printf("the entry在 :%x %d\n",ehdr.e_entry,ehdr.e_entry);
  return ehdr.e_entry;
}


void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]) {
  protect(&pcb->as);
  printf("context_uload执行到%s\n",filename);
  int argc = 0, envc = 0;
  while (argv[argc] != NULL) argc++;
  while (envp[envc] != NULL) envc++;

  char *stack_tmp = (char *)new_page(8); 
  char *stack_pointer = stack_tmp + PGSIZE * 8;
  char *stack_pointer2=stack_pointer-4;

  for (int i = 0; i < 8; ++ i) {
    map(&pcb->as,
        (void*)(pcb->as.area.end - (8 - i) * PGSIZE),
        (void*)(stack_tmp+ i * PGSIZE),
        0);
  }


  uintptr_t argv_pos[argc], envp_pos[envc]; 
  for (int i = 0; i < argc; i++) {
    int len = strlen(argv[i]) + 1;
    stack_pointer2 -= len;
    argv_pos[i] = (uintptr_t)stack_pointer2; 
    strncpy(stack_pointer2, argv[i], len);
  }

  stack_pointer2 = (char *)ROUNDDOWN((uintptr_t)stack_pointer2, sizeof(uintptr_t));
  for (int i = 0; i < envc; i++) {
    int len = strlen(envp[i]) + 1;
    stack_pointer2 -= len;
    envp_pos[i] = (uintptr_t)stack_pointer2; 
    strncpy(stack_pointer2, envp[i], len);
  }
  stack_pointer2 = (char *)ROUNDDOWN((uintptr_t)stack_pointer2, sizeof(uintptr_t));
  size_t ptrs_size = sizeof(uintptr_t) * (1 + argc + 1 + envc + 1);
  stack_pointer2 -= ptrs_size;
  uintptr_t *ptrs = (uintptr_t *)stack_pointer2;
  ptrs[0] = argc;                      
  for (int i = 0; i < argc; i++) {      
    ptrs[i + 1] = argv_pos[i]; 
  }
  ptrs[argc + 1] = 0;                  
  for (int i = 0; i < envc; i++) {     
    ptrs[argc + 2 + i] = envp_pos[i];
  }
  ptrs[argc + envc + 2] = 0;         
  uintptr_t entry = loader(pcb, filename);
  Area stack = RANGE(pcb->stack, pcb->stack + STACK_SIZE);
  pcb->cp = ucontext(&pcb->as, stack, (void *)entry);

  pcb->cp->GPRx = (uintptr_t)ptrs;     
}
void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}