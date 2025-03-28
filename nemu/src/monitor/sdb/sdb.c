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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
void set_watchpoint(char* args);
void delete_watchpoint(int no);
word_t expr(char *e, bool *success);
word_t paddr_read(paddr_t addr, int len);
void sdb_watchpoint_display();
/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
nemu_state.state=NEMU_QUIT;
return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char* args);

static int cmd_w(char *args);

static int cmd_d(char *args);



static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si","si N,Execute the program step by step for N instructions before pausing. When N is not specified, the default is 1", cmd_si},
  { "info","info r print register status; info w print monitoring point information",cmd_info},
  { "x", "x N EXPR, Evaluate the expression EXPR, use the result as the starting memory address, and output the next N 4-byte values in hexadecimal format.",cmd_x},
  { "p", "p EXPR, Evaluate the expression EXPR",cmd_p},
  { "w", "w EXPR, Pause the program execution when the value of expression EXPR change",cmd_w},
  { "d", "d N, Delete the watchpoint with the number N",cmd_d},


  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
  char* arg = strtok(NULL, " ");
  if (arg == NULL) {
    cpu_exec(1);
  } 
  else {
    
    int n ;
     sscanf(arg, "%d", &n);
    cpu_exec(n);
  }
  return 0;
}

static int cmd_info(char *args)
{
char* arg =strtok(NULL," ");
if(arg==NULL)
{	printf("Please enter 'r' or 'w':\n");}
else if(strcmp(arg,"r") == 0)
	isa_reg_display();
else if(strcmp(arg,"w") == 0)
  sdb_watchpoint_display();
return 0;
}

static int cmd_x(char *args) {
    char* arg = strtok(NULL, " ");
    if (arg == NULL) {
        return 0;
    }
   int n = atoi(arg); 
   char *ex = strtok(NULL, " ");
   if (ex == NULL) {
        printf("Please input the right format(1)\n");
        return 0;
    }
    bool flag=true;
    word_t addr_start = expr(ex, &flag);
    if (flag==false)
    {
        printf("Please input the right format(2)\n");
        return 0;
    }
    // if (addr_start % 4 != 0) 
    // {
    //     printf("The starting address must be aligned to 4 bytes.\n");
    //     return 0;
    // }

    // for (int i = 0; i < n; i++) {
    //     word_t value = paddr_read(addr_start, 4);
    //     printf("0x%08X\n", value);
    //     addr_start += 4; 
    // }
    for (int i = 0; i < n; i++) {
        word_t value = paddr_read(addr_start, 4);
        
        printf("0x%08X: ", value);

        for (int j = 3; j >= 0; j--) {
        unsigned char byte = (value >> (j * 8)) & 0xFF;
        printf("\t%d ", byte);
}
        printf("\n");
        addr_start += 4; 
    }

    return 0;
}


static int cmd_p(char* args)
{
    if(args == NULL){
        printf("No args in cmd_p\n");
        return 0;
    }
    bool flag = false;
    word_t result=expr(args, &flag);
    printf("ans is : %u \t%#-8x\n" ,result,result);
    return 0;
}

static int cmd_w(char* args)
{
    set_watchpoint(args);
    return 0;
}

static int cmd_d (char *args)
{
    if(args == NULL)
        printf("No args in cmd_d\n");
    else
    {
        delete_watchpoint(atoi(args));
    }
    return 0;
}




	
void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
