#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
#include <fcntl.h>
int fs_open(const char *pathname, int flags, int mode);
static int evtdev = -1;
static int fbdev = -1;
static int dpfdev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0;
static uint64_t _usec = 0;

typedef struct size
{
  int w;
  int h;
} Size;
Size disp_size;
uint32_t NDL_GetTicks() {
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  uint64_t cur_usec = tv.tv_sec * 1000000 + tv.tv_usec;
  cur_usec -= _usec;
  return cur_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
   
   int ret; 
  int fd = open("/dev/events", 0);
  ret = read(fd, buf, len);
 // close(fd);
  return ret;
}

void NDL_OpenCanvas(int *w, int *h) {
if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    //screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }

  if (*h == 0 && *w == 0) {
    canvas_h = screen_h;
    canvas_w = screen_w;
    *w = screen_w;
    *h = screen_h;
  }
  else if (*h <= screen_h && *w <= screen_w) {
    canvas_h = *h;
    canvas_w = *w;
  }
  else
    printf("canvas oversize the screen size [%d, %d]\n", screen_w, screen_h);
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  //printf("screenw=%d canvas_w=%d screen_h=%d canvas_h=%d\n",screen_w,canvas_w,screen_h,canvas_h);
  x += (screen_w - canvas_w) / 2;
  y += (screen_h - canvas_h) / 2;
  int fd = open("/dev/fb", 0,0);
  //printf("NDL_DrawRect中此时fd是%x\n",fd);
  assert(fd != -1);
  
  //printf("drawing to %d, %x: %d %d %d %d\n", fd, *pixels, x,y,w,h);

  for (int i = 0; i < h; i++) {
    int offset = (y + i)* screen_w + x;
    lseek(fd, offset*4, SEEK_SET);
    write(fd, pixels + (i * w),w*4);
  }
  close(fd);
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
if (getenv("NWM_APP")) {
    evtdev = 3;
  }
// timer-test
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  _usec = tv.tv_sec * 1000000 + tv.tv_usec;

// video test
  int fd=open("/proc/dispinfo",0,0);
  char buf[64];
  int tmp=read(fd,buf,sizeof(buf));
  sscanf(buf, "WIDTH:%d\nHEIGHT:%d\n", &screen_w, &screen_h);

  return 0;
}

void NDL_Quit() {
}
