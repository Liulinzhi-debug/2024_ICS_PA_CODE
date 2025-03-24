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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
word_t expr(char *e, bool *success);
word_t paddr_read(paddr_t addr, int len);
word_t vaddr_read(vaddr_t addr, int len);
enum {
  TK_NOTYPE = 256, TK_EQ,TK_NEQ,TK_NUMBER, TK_AND,TK_OR, 
  TK_HEX, TK_NEG, TK_NOT_MORE,TK_LESS,TK_DEREF, TK_REG ,TK_NO_DEFINE,
  TK_MOD,TK_NOT,TK_ADD,TK_SUB
  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_ADD},         // plus
  {"\\-", TK_SUB},         // sub
  {"\\*", '*'},         // mul
  {"/", '/'},           // div
  {"%", TK_MOD},
  {"==", TK_EQ},        // equal
  {"!=",TK_NEQ},        //not equal
  {"<=", TK_NOT_MORE},  
  {"<", TK_LESS},
  {"!", TK_NOT},
  {"&&", TK_AND},   // && 与操作
  {"\\|\\|", TK_OR},    // || 或操作
  // {"[a-zA-Z]", TK_NO_DEFINE},
  {"(\\$\\w+)|(\\$\\$0)",TK_REG},
  // {"\\$(\\$0|ra|[sgt]p|t[0-6]|s[0-9]|s10|s11|a[0-7])",TK_REG},//register
  {"0[xX][0-9a-fA-F]+", TK_HEX},//hex 
  {"[0-9]+", TK_NUMBER}, //decimal int
  {"\\(", '('},         // l paren
  {"\\)", ')'},         // r paren

};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[12];
} Token;

// static Token tokens[32] __attribute__((used)) = {};
static Token tokens[65536] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        //Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //    i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

          switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          // case TK_NO_DEFINE:
          // Assert(nr_token < 65536,"no define");
          // Assert(substr_len < sizeof(tokens[nr_token].str),"no define");
          // strncpy(tokens[nr_token].str, substr_start, substr_len);
          // tokens[nr_token].str[substr_len] = '\0';
          // tokens[nr_token].type = rules[i].token_type;
          // nr_token++;
          // break;
          case TK_NUMBER:
          case TK_HEX:
          Assert(nr_token < 65536,"token have not enough space");
          Assert(substr_len < sizeof(tokens[nr_token].str),"str have not enough space");
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          tokens[nr_token].type = rules[i].token_type;
          nr_token++;
          break;

          case TK_REG:
          Assert(nr_token < 65536,"token have not enough space");
          Assert(substr_len < sizeof(tokens[nr_token].str),"str have not enough space");
          strncpy(tokens[nr_token].str, substr_start+1, substr_len-1);
          tokens[nr_token].str[substr_len-1] = '\0';
          tokens[nr_token].type = rules[i].token_type;
          nr_token++;
          break;

          default: 
          Assert(nr_token < 65536,"token have not enough space");
          Assert(substr_len < sizeof(tokens[nr_token].str),"str have not enough space");
          tokens[nr_token].type = rules[i].token_type;
          nr_token++;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

//这里是检测括号配对的代码开始
bool check_parentheses(int p, int q) 
{
    int cnt=0;
    int i=0;
    if (tokens[p].type != '(' || tokens[q].type != ')') 
    {
        return false;
    }

    for(i = p ; i <= q ; i ++)
    {    
        if(tokens[i].type == '(' ) cnt++;
        else if(tokens[i].type == ')') cnt--;
        if(cnt == 0 && i < q) return false ;  
    }                              
    if( cnt != 0 ) return false;  
    return true; 
}

//这里是检测括号配对的代码结束
int max(int a,int b)
{
  if(a>b)return a;
  else return b;
}
int min(int a,int b)
{
  if(a<b)return a;
  else return b;
}
//识别取负符号、解引用符号
void take_special_symbol() 
{
    int i;
    for (i = 0; i < nr_token; i++) 
    {
        if (tokens[i].type == TK_SUB){
        if (i == 0 || (tokens[i - 1].type != TK_REG&&tokens[i - 1].type != TK_NUMBER&&tokens[i - 1].type !=TK_HEX && tokens[i - 1].type != ')'))
            {
                tokens[i].type = TK_NEG;
                //printf("取负符号在第 %d 位\n",i+1);
            }
        }
        if (tokens[i].type == '*'){
        if (i == 0 || (tokens[i - 1].type != TK_REG&&tokens[i - 1].type != TK_NUMBER &&tokens[i - 1].type !=TK_HEX && tokens[i - 1].type != ')'))
            {
                tokens[i].type = TK_DEREF;
                //printf("解引用符号在第 %d 位\n",i+1);
            }
        }
        if (tokens[i].type == TK_NOT){
        if (i == 0 || (tokens[i - 1].type != TK_REG&&tokens[i - 1].type != TK_NUMBER &&tokens[i - 1].type !=TK_HEX && tokens[i - 1].type != ')'))
            {
                tokens[i].type = TK_DEREF;
                //printf("逻辑非符号在第 %d 位\n",i+1);
            }
        }
        
    }
}


// //处理取负
// word_t process_negative(int start, int end,word_t result) {

//      // int i=0;
//         //     for(  i = op ; i<nr_token ; i++){
//         //         if(tokens[i].type == TK_NUMBER){
//         //             result = atoi(tokens[i].str);
//         //             //printf("%d \n",result);
//         //             // return -result;
//         //             break;
//         //         }
               
//         //     }  
//         //     for( ;i > 0 ;i --) result = -result;
            
//         //     return result;
//         int i=0;
//         int neg_count = 0; // 初始化负号计数为0
//             for(  i = start ; i<end ; i++){
//               if (tokens[i].type == TK_NEG) 
//                 neg_count++; // 遇到负号，计数加1
//               if(tokens[i].type == TK_NUMBER){
//                 result = atoi(tokens[i].str);
//                     return -result;
//                     break;
//                 }
               
//             }  
//     
// }

int find_main_op(int start,int end)
{

  int found=0;
  int op=-1;
  int p=start;
  // int q=end;
  for(int i=start;i<=end;i++)
     {     
         if(tokens[i].type == '('){
            int cnt=0;
            cnt+= 1;
            i++;
            while(1){
                if(tokens[i].type == '(') cnt += 1;
                else if(tokens[i].type == ')') cnt --;
                
                if(cnt == 0)
                    break;
                i++;
            }  
           }
           

             if(tokens[i].type == TK_OR)
            {
                found = 0;
                op = max(op,i);
            }
             if((found==0||found==1||found == 2||found==3||found==4||found==5||found==6) &&
             tokens[i].type == TK_AND)
            {
                found = 1;
                op = max(op,i);
            }
            if((found==0||found == 2||found==3||found==4||found==5||found==6) &&
            (tokens[i].type == TK_EQ|| tokens[i].type == TK_NEQ||
            tokens[i].type == TK_NOT_MORE||tokens[i].type == TK_LESS))
            {
                found = 2;
                op = max(op,i);
            }


            if((found==0||found==3||found==4||found==5||found==6) &&
            (tokens[i].type == TK_ADD|| tokens[i].type == TK_SUB))
            {
                found = 3;
                op = max(op, i);
            }
            if((found==0||found==4||found==5||found==6) &&
             (tokens[i].type == '*' || tokens[i].type == '/'||tokens[i].type == TK_MOD) )
            {
                found = 4;
                op = max(op, i);
            }
            if((found==0||found==5 ||found==6) && 
            (tokens[i].type == TK_NEG||tokens[i].type == TK_DEREF) )
            {
                found=5;
                op = p;
            }
            if((found==0||found==6 ) && 
            (tokens[i].type == TK_NOT) )
            {
                found=6;
                op = p;
            }
             //printf("op在 %d \n",op+1);
            // printf("%d\n",i);
             //printf("%d\n",tokens[i].type);
     }
      return op;
}

//这里是补充的eval函数开始

word_t eval(int p,int q) 
{
  if (p > q)
  {
    /* Bad expression */
      printf("The input is wrong p>q\n");
      assert(0);
      return -1;
  }
  else if (p == q) 
  {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    // if (tokens[p].type != TK_NUMBER||tokens[p].type !=TK_HEX)
    // {printf("illegal symbol:%d\n", p);return 0;}
    // if (tokens[p].type !=TK_NO_DEFINE)
    // {
    //   printf("the token is wrong:%d\n", p);
    //         return 0;
    // }
        if (tokens[p].type != TK_NUMBER && tokens[p].type != TK_HEX && tokens[p].type != TK_REG)
        {   
            printf("the token is wrong:%d\n", p);
            return 0;
        }
        word_t num = 0;
        if (tokens[p].type == TK_REG) 
        {
            bool flag = true;
            num = isa_reg_str2val(tokens[p].str, &flag);
            if (!flag) 
            {
                num = 0;
            }
             return num;
        }
        else if (tokens[p].type == TK_HEX) 
         {  // int tmp=0;
            if(tokens[p].str[0]=='0'&&(tokens[p].str[1]=='x'||tokens[p].str[1]=='X'))
            {
              num=strtol(tokens[p].str,NULL,16);
            // for(int i = 2;i < strlen(tokens[p].str); i++)
            // {  
            //    if(tokens[p].str[i]<='9')
            //      {
            //       tmp=tokens[p].str[i]-'0';
            //      }
            //    else
            //    {
            //      if(tokens[p].str[i]>='a'&&tokens[p].str[i]<='z')
            //     {
            //      tmp=tokens[p].str[i]-'a'+10;
            //     }
            //      else if(tokens[p].str[i]>='A'&&tokens[p].str[i]<='Z')
            //     {
            //      tmp=tokens[p].str[i]-'A'+10;
            //     }
            //    }
            //    num = num* 16 + tmp;
            // }
            }
            return num;
        }
        
        else if(tokens[p].type == TK_NUMBER)
        {
        //     for (int i = 0; i < strlen(tokens[p].str); i++)
        //         num = num * 10 + tokens[p].str[i] - '0';
        num=atoi(tokens[p].str);
        return num;
        }

  }
  else if (check_parentheses(p, q) == true) 
  {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }
  else 
  {
   // op = the position of 主运算符 in the token expression;
    int op = -1; // 主运算符
    // int found = 0;//寻找优先级
        // for(int i = p ; i <= q ; i ++)
        // {
          //  int cnt=0;
          //  if(tokens[i].type == '('){
          //   cnt+= 1;
          //   i++;
          //   while(1){
          //       if(tokens[i].type == '(') cnt += 1;
          //       else if(tokens[i].type == ')') cnt --;
          //       i++;
          //       if(cnt == 0)
          //           break;
          //   }  
          //  }
             op=find_main_op(p,q);
            //   if(tokens[i].type == TK_OR)
            // {
            //     found = 0;
            //     op = max(op,i);
            // }
            //  if((found==0||found==1||found == 2||found==3||found==4||found==5||found==6) &&
            //  tokens[i].type == TK_AND)
            // {
            //     found = 1;
            //     op = max(op,i);
            // }
            // if((found==0||found == 2||found==3||found==4||found==5||found==6) &&
            // (tokens[i].type == TK_EQ|| tokens[i].type == TK_NEQ||
            // tokens[i].type == TK_NOT_MORE||tokens[i].type == TK_LESS))
            // {
            //     found = 2;
            //     op = max(op,i);
            // }


            // if((found==0||found==3||found==4||found==5||found==6) &&
            // (tokens[i].type == '+' || tokens[i].type == '-'))
            // {
            //     found = 3;
            //     op = max(op, i);
            // }
            // if((found==0||found==4||found==5||found==6) &&
            //  (tokens[i].type == '*' || tokens[i].type == '/'||tokens[i].type == TK_MOD) )
            // {
            //     found = 4;
            //     op = max(op, i);
            // }
            // if((found==0||found==5 ||found==6) && 
            // (tokens[i].type == TK_NEG||tokens[i].type == TK_DEREF) )
            // {
            //     found=5;
            //     op = p;
            // }
            // if((found==0||found==6 ) && 
            // (tokens[i].type == TK_NOT) )
            // {
            //     found=6;
            //     op = p;
            // }
        // }
      
      word_t  val1 ;
      word_t  val2 ;     
      int op_type = tokens[op].type;
      if(op_type == TK_NEG||op_type == TK_DEREF||op_type ==TK_NOT)
      {
        if(op_type == TK_NEG)
        {
        word_t result=eval(op + 1, q);  
        return -result;
        }
        else if(op_type == TK_DEREF)
        {
        word_t addr=eval(op + 1, q);  
        // char hex_str[32];
        // sprintf(hex_str, "0x%X", result); 
        // return *hex_str;
        word_t result = paddr_read(addr, 4);
        return result;
        }
        else if(op_type == TK_NOT)
        {
        
        word_t result = eval(op + 1, q);
        return !result;
        }
      }
    else
    {
     val1 = eval(p, op - 1);
     val2 = eval(op + 1, q);   
    switch (op_type) 
    {
      case TK_ADD: return val1 + val2;
      case TK_SUB: return val1 - val2;
      case '*': return val1 * val2;
      case '/': 
       
        Assert(val2 != 0,"div is zero\n");
        return val1 / val2;
      case TK_EQ: return val1 == val2;
      case TK_NEQ:return val1 != val2;
      case TK_LESS:return val1 < val2;
      case TK_NOT_MORE: return val1 <= val2;
      case TK_AND:return val1 && val2;
      case TK_OR :return val1 || val2;
      case TK_MOD:return val1 % val2;
      default: 
      printf("illegal symbol\n");
      assert(0);
    }
    }
  }
  return 0;
}


// //这里是补充的eval函数结束




word_t expr(char *e, bool *success) 
{

  word_t exprAns;

  if (!make_token(e)) {
    *success = false;
    return 0;
  }
//更改开头
  /* TODO: Insert codes to evaluate the expression. */



  take_special_symbol();
  exprAns = eval(0, nr_token - 1);

  *success = true;
  return exprAns;
  }

//更改结尾

