#include <common.h>
#include "syscall.h"
#include <fs.h>
#include <proc.h>
void display_strace(uintptr_t a7, uintptr_t a0, uintptr_t a1, uintptr_t a2);
void display_strace(uintptr_t a7, uintptr_t a0, uintptr_t a1, uintptr_t a2) {
    printf("执行到do_syscall\t c->GPR1=a7=%d, c->GPR2=a0=%d, c->GPR3=a1=%d, c->GPR4=a2=%d\n", a7, a0, a1, a2);
}
int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
int fs_close(int fd);
size_t fs_lseek(int fd, size_t offset, int whence);
size_t fs_write(int fd, const void *buf, size_t len);
typedef struct timeval
{
  int32_t tv_sec;  /* seconds */
  int32_t tv_usec; /* microseconds */
} Timeval;
typedef struct timezone
{
  int tz_minuteswest; /* minutes west of Greenwich */
  int tz_dsttime;     /* type of DST correction */
} Timezone;
static int gettimeofday(Timeval *tv, Timezone *tz);
void naive_uload(PCB *pcb, const char *filename);
int sys_execve(const char *pathname, char *const argv[], char *const envp[]);
void exit(int status);
void switch_boot_pcb();
int mm_brk(uintptr_t brk);
void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]);

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

 
  //display_strace(a[0], a[1], a[2], a[3]); 



  //printf("执行到do_syscall\t c->GPR1=a7=%d\n",a[0]);
  switch (a[0]) {
   // case 0:c->GPRx=0;printf("SYS_exit， do_syscall此时 c->GPRx=%d\n",c->GPRx);halt(c->GPRx);
   // case 1:printf("SYS_yield， do_syscall此时c->GPRx=%d\n",c->GPRx);yield();
  case SYS_exit:

    exit((int)c->GPR2);
    break;
  case SYS_yield:
  
    printf("SYS_yield\tdo_syscall\tc->GPRx=%d\n",c->GPRx);
    yield();
    c->GPRx = 0;
    break;
  case SYS_open:
    c->GPRx = fs_open((const char *)a[1], (int)a[2], (int)a[3]);
    break;
  case SYS_write:
    c->GPRx = fs_write((int)c->GPR2, (const void *)c->GPR3, (size_t)c->GPR4);
    break;
  case SYS_read:
    c->GPRx = fs_read((int)a[1], (void *)a[2], (size_t)a[3]);
    break;
  case SYS_lseek:
    c->GPRx = fs_lseek((int)c->GPR2, (size_t)c->GPR3, (int)c->GPR4);
    break;
  case SYS_close:
    c->GPRx = fs_close((int)a[1]);
    break;
  case SYS_execve:
   c->GPRx = sys_execve((const char *)c->GPR2, (char **const)c->GPR3, (char **const)c->GPR4);
    //while(1);
    break;
  case SYS_brk:
    c->GPRx = (int)mm_brk((uintptr_t)c->GPR2);
    break;
  case SYS_gettimeofday:
    c->GPRx = gettimeofday((Timeval *)c->GPR2, NULL);
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
  
}



static int gettimeofday(Timeval *tv, Timezone *tz)
{

    assert(tv != NULL);
  uint64_t us = io_read(AM_TIMER_UPTIME).us;
  tv->tv_sec = us / 1000000;
  tv->tv_usec = us % 1000000;
  return 0;
}
int sys_execve(const char *pathname, char *const argv[], char *const envp[])
{
  
  printf("execve: execve pathname is %s\n", pathname);
  int fd = fs_open(pathname, 0, 0);
  if (fd == -1)
  {
    return -2;
  }
  else
  {
    fs_close(fd);
  }
  //naive_uload(NULL, pathname);
  context_uload(current, pathname, argv, envp);
  switch_boot_pcb();
  yield();
  return 0;
}
 
 void exit(int status)
{
  if (status == 0)
  {
    sys_execve("/bin/nterm", 0, 0);
  }
  else
    halt(status);
    
}