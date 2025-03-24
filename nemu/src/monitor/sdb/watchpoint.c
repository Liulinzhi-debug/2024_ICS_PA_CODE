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

#include "sdb.h"

#define NR_WP 32
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  word_t value;
  char express[128];
} WP;
WP* new_wp();
void free_wp(WP *wp);
static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP *new_wp()
{
// Assert(head!=NULL,"the watchpoint is empty");
Assert(free_!=NULL,"the watchpoint is FULL!");
WP* cur =free_;
free_=free_->next;
if(head==NULL)
{
 head=cur;
 head->next=NULL;
}
else 
{
  cur->next=head;
  head=cur;
}
return head;
}

void free_wp(WP *wp)
{
  Assert(head!=NULL,"the watchpoint is empty");
// Assert(free_!=NULL,"the watchpoint is FULL!");
   if(head -> NO == wp -> NO)
   {
        WP* cur=head;
        head=head->next;
        cur->next=free_;
        free_=cur;
        return ;
    }
    else 
    {
     WP *l=head;
     WP *r=l->next;
     while(r->NO!=wp->NO&&r!=NULL)  
     {
      l=r;
      r=r->next;
     }
     if(r->NO==wp->NO&&r!=NULL)
     {
      l->next=r->next;
      r->next=free_;
      free_=r;
      printf("已经删除第%d个监视点\n", wp->NO);
        return;
     }
     printf("不存在第%d个监视点\n",wp->NO);
     return;
    }
}

void sdb_watchpoint_display()//输出监视点
{
  WP* cur = head;
  if (cur== NULL)
  {
    printf("the watchpoint is empty\n");
    return ;
  }
  else{
        while(cur!=NULL)
        {
  
            printf("Watchpoint.No:%d\t The express:%s\t  The value of express: %#x\n",cur->NO , cur->express, cur->value);
            cur=cur->next;
        }
        return;
    }
    return;
}

void newvalue_cmp_oldvalue()
{
WP* cur=head;
word_t newvalue;
bool flag=true;
while(cur!=NULL)
{
  Assert(cur->express!=NULL,"the watchpoint has no express");
  newvalue=expr(cur->express,&flag);
  Assert(flag==true,"illegal express");
  if(cur->value==newvalue)
  {cur=cur->next;continue;}
  else if(cur->value!=newvalue)
  {
   // nemu_state.state = NEMU_STOP;
    printf("Old value = %u\t %#08x\n", cur->value,cur->value);
    printf("New value = %u\t %#08x\n", newvalue,newvalue);
    printf("Hardware watchpoint :%d \n", cur->NO);
    printf("New express : %s \n",cur->express);
    cur->value=newvalue;
    cur=cur->next;
  }
}

}

void set_watchpoint(char* args)
{
WP* cur=new_wp();
Assert(cur!=NULL,"the watchpoints are full");
// cur->express=args;//存入表达式
strcpy(cur->express, args);
bool flag=true;
cur->value  =expr(args,&flag);
Assert(flag==true,"illegal express in create_watchpoint");
//if(flag==true)printf("illegal express in create_watchpoint");
printf("Hardware watchpoint %d: %s\n", cur->NO, args);
}



void delete_watchpoint(int no)
{
  WP* cur =head;
  while(cur!=NULL)
  {
   if(cur->NO==no)
   break;
   else
   cur=cur->next;
  }
    if(cur==NULL)
    {
      printf("the watchpoint is not busy or does not exist\n");
      return;
    }
    free_wp(cur);
    return;
}