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
#include <cpu/difftest.h>
#include "../local-include/reg.h"

extern const char* regs[];
bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {

for (int i = 0; i < 32; i++) 
{
    if (ref_r->gpr[i] != cpu.gpr[i])
    {
      printf("[%s] should be 0X%08x ,but get 0X%08x\n", regs[i], ref_r->gpr[i]._32, cpu.gpr[i]._32);
      printf("PC should be 0X%08x,  get 0X%08xn", ref_r->pc, pc);
      return false;
    }
}
 if(ref_r->pc == pc)
		return true;
	else {
		printf("PC should be 0X%08x,  but get 0X%08xn", ref_r->pc, pc);
// 		return false;
}



 return false;
}

void isa_difftest_attach() {
}
