#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static char out_buf[1024];

static char num_16[] = "0123456789ABCDEF";

int my_int_to_str(char *str,unsigned int num,int uflag,int base);


int printf(const char *fmt, ...) {
  //panic("Not implemented");
    va_list args;
    va_start(args, fmt);
    //putstr(out_buf);
    //putstr(fmt);

    int result = vsprintf(out_buf, fmt, args); 
    putstr(out_buf);
    va_end(args);

    return result;
  
}

int vsprintf(char *out, const char *fmt, va_list args) {
   // panic("Not implemented");
    //putstr(fmt);
    int i = 0, j = 0;
    char *p = out;

    while (fmt[i] != '\0') {
        if (fmt[i] == '%') {
            i++;
            switch (fmt[i]) {
                case 'd':
                case 'i': {
                    int num = va_arg(args, int);
                    p = out + j;
                    j += my_int_to_str(p, num,0,10);
                    break;
                }
                  case 'u': {
                    unsigned int num = va_arg(args, int);
                    p = out + j;
                    j += my_int_to_str(p, num,1,10);
                    break;
                }
                   case 'x': {

                    unsigned int num = va_arg(args, int);
                    p = out + j;
                    j += my_int_to_str(p, num,1,16);
                    break;
                }
                case 's': {
                    char *str = va_arg(args, char *);
                    while (*str) 
                    {
                        out[j++] = *str++;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
				    out[j++] = c;
				
                    break;
                }
                
                default:
                    out[j++] = fmt[i];
                    break;
            }
        } else {
            out[j++] = fmt[i];
        }
        i++;
    }
    out[j] = '\0';
    return j;
    
}





int my_int_to_str(char *str,unsigned int num,int uflag,int base) {
    int i = 0;
    int isNegative = 0;

    if (num < 0&&uflag==0) {
        isNegative = 1;
        num = -num;
    }

    if (num == 0) {
        str[i++] = '0';
    } 
    else {
        while (num != 0) {
            str[i++] = num_16[num % base];
            num = num / base;
        }
    }

    if (isNegative) {
        str[i++] = '-';
    }
    if(base==16)
    {
        str[i++]='X';
        
        str[i++]='0';
    }
    str[i] = '\0';

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }

    return i;
}


int sprintf(char *out, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int i = 0, j = 0;
    char *p = out;

    while (fmt[i] != '\0') {
        if (fmt[i] == '%') {
            i++;
            switch (fmt[i]) {
                case 'd':
                case 'i': {
                    int num = va_arg(args, int);
                    p = out + j;
                    j += my_int_to_str(p, num,0,10);
                    break;
                }
                case 's': {
                    char *str = va_arg(args, char *);
                    while (*str) {
                        out[j++] = *str++;
                    }
                    break;
                }
                 case 'x': {
                    unsigned int num = va_arg(args, int);
                    p = out + j;
                    j += my_int_to_str(p, num,1,16);
                    break;
                }
                case 'u': {
                    unsigned int num = va_arg(args, int);
                    p = out + j;
                    j += my_int_to_str(p, num,1,10);
                    break;
                }
                default:
                    out[j++] = fmt[i];
                    break;
            }
        } else {
            out[j++] = fmt[i];
        }
        i++;
    }
    out[j] = '\0';
    va_end(args);
    return j;
}






int snprintf(char *out, size_t n, const char *fmt, ...) {
  //panic("Not implemented");
   va_list args;
    va_start(args, fmt);
    int result = vsnprintf(out, n, fmt, args);
    va_end(args);
    return result;
  
}
int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
 //panic("Not implemented");
   int i = 0, j = 0;
    char *p = out;

    size_t max_len = n - 1;

    while (fmt[i] != '\0' && j < max_len) {
        if (fmt[i] == '%') {
            i++;  
            switch (fmt[i]) {
                case 'd':
                case 'i': {
                    int num = va_arg(ap, int);
                    p = out + j;
                    j += my_int_to_str(p, num, 0, 10);
                    break;
                }
                case 'u': {
                    unsigned int num = va_arg(ap, unsigned int);
                    p = out + j;
                    j += my_int_to_str(p, num, 1, 10);
                    break;
                }
                case 'x': {
                    unsigned int num = va_arg(ap, unsigned int);
                    p = out + j;
                    j += my_int_to_str(p, num, 1, 16);
                    break;
                }
                case 's': {
                    char *str = va_arg(ap, char *);
                    while (*str && j < max_len) {
                        out[j++] = *str++;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(ap, int);
                    if (j < max_len) {
                        out[j++] = c;
                    }
                    break;
                }
                default:
                    if (j < max_len) {
                        out[j++] = fmt[i];
                    }
                    break;
            }
        } else {
            if (j < max_len) {
                out[j++] = fmt[i];
            }
        }
        i++;
    }


    out[j] = '\0';

 
    return j;
}



#endif
