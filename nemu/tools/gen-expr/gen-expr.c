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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>


static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; 
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";




// //这里是修改后的关于随机生成表达式的代码开头
static int buf_index=0;



int choose(int n)
{
    int ran = rand() % n; // 0 1 2
   // printf("idx = %d, ran_num = %d. \n",buf_index, ran);
    return ran;
}


static void gen(char c) {
    buf[buf_index++] = c;
    buf[buf_index] = '\0'; // 确保字符串以 null 结尾
}

static void gen_num() 
{
    int ran_num=rand()%100;
    char a = ran_num%10+'0';
    ran_num/=10;
    if((ran_num)!=0)
    {
        char b = ran_num+'0';
        gen(b);
        gen(a);
    }
    else 
    gen(a);
    
}
// static void gen_space() {
//     int len = choose(4); // 生成 0 到 3 个空格
//     for (int i = 0; i < len; i++) {
//         gen(' ');
//     }
// }
static void gen_neg() {
    int len = choose(4); // 生成 0 到 3 个空格
    for (int i = 0; i < len; i++) {
        gen('-');
    }
}

static void gen_rand_op() {
    char op[] = {'+', '-', '*'};
    gen(op[choose(3)]); // 随机选择一个运算符
}
// static void gen_rand_expr() {
//   buf[0] = '\0';
// }//原本 gen_rand_expr()便于恢复初始状态

static  void gen_rand_expr() 
{
  // switch (choose(3)) 
  // {
  //   case 0: gen_num(); break;
  //   case 1: gen('('); gen_rand_expr(); gen(')'); break;
  //   default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  // }
  if (buf_index >100) {
        printf("oversize\n");
        gen_num();
        return;
    }

    switch (choose(3)) 
    {
        case 0:
            // gen_space();
            gen_neg();
            gen_num();
            //gen_space();
            break;
        case 1:
            gen('(');
            gen_rand_expr();
            gen(')');
            break;
        default:
            gen_rand_expr();
            gen_rand_op();
            gen_rand_expr();
            break;
  }

}

  //  buf[index_buf] = '\0';


 
/*Generate a random number less than n*/
//这里是修改后的关于随机生成表达式的代码结尾


int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    buf_index=0;
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
