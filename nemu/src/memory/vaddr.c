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
#include <memory/paddr.h>

// word_t vaddr_ifetch(vaddr_t addr, int len) {
//   return paddr_read(addr, len);
// }

word_t vaddr_ifetch(vaddr_t addr, int len)
{
  //Log("addr 是:%x\n",addr);
  int mmu_type = isa_mmu_check(addr, len, MEM_TYPE_IFETCH);
  paddr_t paddr;
  if (mmu_type == MMU_DIRECT)
  {
    //printf("iam here 1 \n");
    paddr = addr;
  }
  else if (mmu_type == MMU_TRANSLATE) 
  {
    //printf("iam here 2 \n");
    //printf("in vaddr_ifetch addr是%x\n",addr);
    paddr = isa_mmu_translate(addr, len, MEM_TYPE_IFETCH);
  } 
  else assert("vaddr_ifetch 错误");
  //assert(paddr == addr);
  return paddr_read(paddr, len);
}

// word_t vaddr_read(vaddr_t addr, int len) {
//   return paddr_read(addr, len);
// }
word_t vaddr_read(vaddr_t addr, int len) {
  //Log("addr 是:%x\n",addr);
  int mmu_type = isa_mmu_check(addr, len, MEM_TYPE_READ);
  paddr_t paddr;
  if (mmu_type == MMU_DIRECT)
  {
    //printf("iam here 1 \n");
    //printf("in vaddr_read MMU_DIRECT addr是%x\n",addr);
    paddr = addr;
  }
  else if (mmu_type == MMU_TRANSLATE)
  {
    //printf("iam here 2 \n");
    //printf("in vaddr_read MMU_TRANSLATE addr是%x\n",addr);
    paddr = isa_mmu_translate(addr, len, MEM_TYPE_READ);
  } 
  else panic("vaddr_read错误");
  //assert(paddr == addr);
  return paddr_read(paddr, len);
}

// void vaddr_write(vaddr_t addr, int len, word_t data) {
//   paddr_write(addr, len, data);
// }
void vaddr_write(vaddr_t addr, int len, word_t data) {
  int mmu_type = isa_mmu_check(addr, len, MEM_TYPE_WRITE);
  paddr_t paddr;
  if (mmu_type == MMU_DIRECT)
  {
    //printf("iam here 1 \n");
    paddr = addr;
  }
  else if (mmu_type == MMU_TRANSLATE)
   {
    //printf("iam here 2 \n");
     //printf("in vaddr_write addr是%x\n",addr);
    paddr = isa_mmu_translate(addr, len, MEM_TYPE_WRITE);
  }
   else panic("vaddr_write错误");
  //assert(paddr == addr);
  paddr_write(paddr, len, data);
}