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
#include <memory/vaddr.h>
#include <memory/paddr.h>

// paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
//   return MEM_RET_FAIL;
// }

#define VA_OFFSET(vaddr) (vaddr & 0xfff)
#define VA_VPN_0(vaddr)  (((vaddr & 0x3fffff) >> 12)& 0x3ff)
#define VA_VPN_1(vaddr)  ((vaddr >> 22) & 0x3ff) 
 
#define PTE_V(item)   (item & 0x1)
#define PTE_R(item)   ((item >> 1) & 0x1)
#define PTE_W(item)   ((item >> 2) & 0x1)
#define PTE_X(item)   ((item >> 3) & 0x1)
#define PTE_PPN(item) ((item >> 12) & 0xfffff) 
#define VPN_SHIFT 12
#define SATP_PPN_MASK 0x3FFFFF  
typedef vaddr_t PTE;

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {

  uint32_t satp = cpu.csr.satp;

  PTE page_dir_base = (satp & SATP_PPN_MASK) << VPN_SHIFT;

  uint32_t offset = VA_OFFSET(vaddr);
  uint32_t vpn_1 = VA_VPN_1(vaddr);
  uint32_t vpn_0 = VA_VPN_0(vaddr);
  //Log("isa_mmu_translate中");
  //printf("va 是 %x offset 是 %x vpn_1 是 %x vpn_0 是 %x\n",vaddr,offset,vpn_1,vpn_0);
  PTE page_dir_target = page_dir_base + (vpn_1 <<2);
  //printf("page_dir_target 是 %x\n",page_dir_target);
  uint32_t page_dir_target_item = paddr_read(page_dir_target, 4);
  // printf("isa_mmu_translate中va是%x\n",vaddr);
  
  if (PTE_V(page_dir_target_item) == 0) {
    printf("一级页表项无效: vaddr=0x%x, page_dir_target=0x%x, item=0x%x\n",
           vaddr, page_dir_target, page_dir_target_item);
    assert(0);
  }

  //printf("一级页表项有效: vaddr=0x%x, page_dir_target=0x%x, item=0x%x\n",
  //           vaddr, page_dir_target, page_dir_target_item);
  PTE page_table_base = PTE_PPN(page_dir_target_item) << 12;
  PTE page_table_target = page_table_base + (vpn_0 <<2);
  uint32_t page_table_target_item = paddr_read(page_table_target, 4);

  if (PTE_V(page_table_target_item) == 0) {
    printf("二级页表项无效: vaddr=0x%x, page_table_target=0x%x, item=0x%x\n",
           vaddr, page_table_target, page_table_target_item);
    assert(0);
  }

  // switch (type) {
  //   case MEM_TYPE_IFETCH:
  //     if (PTE_X(page_table_target_item) == 0) {
  //       printf("无执行权限: vaddr=0x%x, item=0x%x\n", vaddr, page_table_target_item);
  //       assert(0);
  //     }
  //     break;
  //   case MEM_TYPE_READ:
  //     if (PTE_R(page_table_target_item) == 0) {
  //       printf("无读权限: vaddr=0x%x, item=0x%x\n", vaddr, page_table_target_item);
  //       assert(0);
  //     }
  //     break;
  //   case MEM_TYPE_WRITE:
  //     if (PTE_W(page_table_target_item) == 0) {
  //       printf("无写权限: vaddr=0x%x, item=0x%x\n", vaddr, page_table_target_item);
  //       assert(0);
  //     }
  //     break;
  //   default:
  //     printf("未知访问类型: type=%d\n", type);
  //     assert(0);
  // }

  paddr_t ppn = PTE_PPN(page_table_target_item) << 12;
  paddr_t paddr = ppn | offset;
  //printf("isa_mmu_translate有效，paddr是%x\n",paddr);
  return paddr;
}

