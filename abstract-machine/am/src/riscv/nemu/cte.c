#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>
#include <nemu.h>
static Context* (*user_handler)(Event, Context*) = NULL;
void display_etrace(Context *c){
  printf("MCAUSE是：%x\tMSTATUS是： %x\tMEPC是： %x\n",c->mcause,c->mstatus,c->mepc);
}
void __am_get_cur_as(Context *c);
void __am_switch(Context *c);
Context* __am_irq_handle(Context *c) {
   //printf("%x %x\n",0xffffffff,0x0);
   //printf("%x %x %x\n",c->mcause,c->mstatus,c->mepc);
 __am_get_cur_as(c);
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
    case (size_t)-1:
       ev.event = EVENT_YIELD;
        c->mepc = c->mepc + 4;
       break;
    case (size_t) 0: 
    case (size_t) 1: 
    case 2: 
    case 3: 
    case 4: 
    case 5: 
    case 6: 
    case 7: 
    case 8: 
    case 9: 
    case 10: 
    case 11: 
    case 12: 
    case 13: 
    case 14: 
    case 15: 
    case 16: 
    case 17: 
    case 18: 
    case 19:       
      ev.event = EVENT_SYSCALL; 
      c->mepc = c->mepc + 4;
      break;
    case 0x80000007:
      ev.event = EVENT_IRQ_TIMER;break;  
    default: 
      printf("__am_irq_handle\tev.event是%d\n",ev.event);
      ev.event = EVENT_ERROR; 
      break;
    }
    //printf("user_handled, context=%x\n", c);
    c = user_handler(ev, c);
    assert(c != NULL);
  }
  //if(c->mcause==0x80000007)
   //c->mepc = c->mepc + 4;
   //display_etrace(c);
  __am_switch(c);
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  //return NULL;
  Context *ctx = (Context *)((uint8_t *)(kstack.end) - sizeof(Context));
  memset(ctx, 0, sizeof(ctx));
  ctx ->mepc = (uintptr_t)entry;
  ctx->gpr[10] = (uintptr_t)arg;
  ctx->mstatus = 0x80;
  //ctx->mstatus = 0x1800;
  ctx->mcause = 0;
  ctx->GPRx = (uintptr_t)arg;
  ctx->np= 0;
  //ctx->gpr[2] = (uintptr_t)kstack.end ;
  return ctx;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
