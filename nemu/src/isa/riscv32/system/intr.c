/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

#define IRQ_TIMER 0x80000007 
#define MSTATUS_MPIE 0x00000080  
#define MSTATUS_MIE  0x00000008  

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
   
    cpu.csr.mepc = epc;
    cpu.csr.mcause = NO;

    // 保存当前 MIE 到 MPIE，然后关闭中断（MIE=0）
    if (cpu.csr.mstatus & MSTATUS_MIE) {
        cpu.csr.mstatus |= MSTATUS_MPIE;   // MIE=1 → MPIE=1
//printf("MIE==1-->MPIE==1\n");
    }
     else {
        cpu.csr.mstatus &= (~MSTATUS_MPIE);  // MIE=0 → MPIE=0
//printf("MIE==0-->MPIE==0\n");
    }
    cpu.csr.mstatus &= (~MSTATUS_MIE);       // 关闭中断（MIE=0）
//printf(" 关闭中断（MIE=0）\n");
    return cpu.csr.mtvec;  // 返回中断处理程序入口地址
}

// word_t isa_query_intr() {
//   return INTR_EMPTY;
// }

word_t isa_query_intr() {
    if (((cpu.csr.mstatus & MSTATUS_MIE)== MSTATUS_MIE)&& (cpu.INTR)) { 
        cpu.INTR = false;
        //printf("cpu.INTR is %d\n", cpu.INTR);
        return IRQ_TIMER;
    }
    return INTR_EMPTY; 
}


void restore_interrupt() { 
  // 还原之前保存的 MPIE 到 MIE
  if((cpu.csr.mstatus & MSTATUS_MPIE) == MSTATUS_MPIE)//MPIE位是1
  {cpu.csr.mstatus |= MSTATUS_MIE;
  //printf("MPIE==1-->MIE==1\n");
  }
  else                                              //MPIE位是0
  {cpu.csr.mstatus = (cpu.csr.mstatus)&(~(MSTATUS_MIE));
   //printf("MPIE==0-->MIE==0\n");
  }
  // 将 MPIE 位置为 1
  cpu.csr.mstatus = (cpu.csr.mstatus|(MSTATUS_MPIE));
  //printf("将 MPIE 位置为 1\n");
  //printf("restore_interrupt cpu.csr.mstatus是%x\n",cpu.csr.mstatus);
 
}
