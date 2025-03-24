#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  //panic("Not implemented");
  if(s==NULL)
  {
    return 0;
  }
  const char *head = s;
  size_t len=0;
  while (*head!='\0') {
      head++;
      len++;
  }
  return len;
}

char *strcpy(char *dest, const char *src) {
  //panic("Not implemented");
  size_t cur=0;
    if (dest == NULL || src == NULL) 
    {  
    return dest;
    }
    for(;src[cur]!='\0';cur++)
    {
      dest[cur]=src[cur];
    }
    dest[cur]='\0';
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
  //panic("Not implemented");
      size_t cur=0;
    if (dest == NULL || src == NULL) 
    {  
    return dest;
    }
    for(;src[cur]!='\0'&&cur<n;cur++)
    {
      dest[cur]=src[cur];
    }
    dest[cur]='\0';
    
    return dest;
}

char *strcat(char *dst, const char *src) {
  //panic("Not implemented");
  size_t cur = 0;
  while(dst[cur] != '\0'){
    cur++;
  }
	 
  strcpy(dst + cur, src);
  return dst;
  
}

int strcmp(const char *s1, const char *s2) {
  //panic("Not implemented");
  size_t i = 0;
  while(s1[i] != '\0' && s2[i] != '\0')
  {
	  if(s1[i] > s2[i])
		  return 1;
	  if(s1[i] < s2[i])
		  return -1;
	  i++;
  }
  if(s1[i] != '\0' && s2[i] == '\0')
	  return 1;
  if(s1[i] == '\0' && s2[i] != '\0')
	  return -1;

  return 0;

}

int strncmp(const char *s1, const char *s2, size_t n) {
  //panic("Not implemented");
   size_t i = 0;
  while(i<n)
  {
	  if(s1[i] > s2[i])
		  return 1;
	  if(s1[i] < s2[i])
		  return -1;
	  i++;
  }
  return 0;
}

void *memset(void *s, int c, size_t n) {
  //panic("Not implemented");
    unsigned char *p = (unsigned char *)s;
    for (size_t i = 0; i < n; i++) {
        p[i] = (unsigned char)c;
    }
    
    return s;

}

void *memmove(void *dest, const void *src, size_t n) {
  //panic("Not implemented");
  unsigned char *p;
  if (dest < src) 
  {
        // 从低地址向高地址复制
        p = (unsigned char *)dest;
        const unsigned char *s = (const unsigned char *)src;
        for (size_t i = 0; i < n; i++) {
            p[i] = s[i];
        }
    } 
    else {
        // 从高地址向低地址复制
        p = (unsigned char *)dest + n-1;
        const unsigned char *s = (const unsigned char *)src + n-1;
        for (size_t i = 0; i < n; i++) {
            p[-i] = s[-i];
        }
    }
    return dest;
}

void *memcpy(void *out, const void *in, size_t n) {
 // panic("Not implemented");
    unsigned char *pout = (unsigned char *)out;
    const unsigned char *pin = (const unsigned char *)in;
    for (size_t i = 0; i < n; i++) {
        pout[i] = pin[i];
    }
    return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  //panic("Not implemented");
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    for (size_t i = 0; i < n; i++) {  
        if (p1[i] > p2[i]) return 1;
        if (p1[i] < p2[i]) return -1;
    }
    return 0;
}

#endif
