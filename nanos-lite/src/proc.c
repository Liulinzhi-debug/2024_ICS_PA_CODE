#include <proc.h>

#define MAX_NR_PROC 4
void naive_uload(PCB *pcb, const char *filename);
void context_kload(PCB* pcb, void(*entry)(void*), void* arg);
static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;
//void context_uload(PCB *pcb, const char *filename);
void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]);
void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    //if (j % 100 == 0)
    //Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (char *)arg, j);
    if (j % 1 == 0)
    printf("Hello World from Nanos-lite with arg '%s' for the %dth time!\n", (char *)arg, j/100000);
    j ++;
    yield();
  }
}
static int cur_pcb=1;
void shift_pcb(int fg_pcb)
{
  if(cur_pcb == fg_pcb)return;
  switch_boot_pcb();
  cur_pcb = fg_pcb;
  pcb[0].cp->pdir = NULL;
  //printf("1\n");
  yield();
}
void init_proc() {
   Log("Initializing processes...");

  context_kload(&pcb[0], hello_fun,"A");
  char* test_argv[] = { NULL };
  char* test_envp[] = {NULL};

  context_uload(&pcb[1], "/bin/pal", test_argv, test_envp);
  context_uload(&pcb[2], "/bin/menu", test_argv, test_envp);
  context_uload(&pcb[3], "/bin/nslider", test_argv, test_envp);

  switch_boot_pcb();


}

// Context* schedule(Context *prev) {
//   //return NULL;
//   current->cp = prev;
//   //current = &pcb[0];
//   current = ((current == &pcb[0]) ? &pcb[1] : &pcb[0]);
//   return current->cp;
// }
Context* schedule(Context *prev) {

  current->cp = prev;
  static int pal_schedule_count = 0;

  if (current == &pcb[0]) {
    current = &pcb[cur_pcb] ;          
    pal_schedule_count = 0;     
  }

  else {
    pal_schedule_count++;
    if (pal_schedule_count >= 1) {
      current = &pcb[0];        
      pal_schedule_count = 0;   
    } else {
      current = &pcb[cur_pcb] ;        
    }
  }

  return current->cp;
}
void context_kload(PCB* pcb, void(*entry)(void*), void* arg) {
 // protect(&pcb->as);
  Area kstack = RANGE(pcb, (char *)pcb + STACK_SIZE);
  pcb->cp = kcontext(kstack, entry, arg);
  
}


