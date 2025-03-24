#include <common.h>
#include<amdev.h>
#include <proc.h>
#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};
void shift_pcb(int fg_pcb);
size_t serial_write(const void *buf, size_t offset, size_t len) {
  //yield();
  char *str = (char *)buf;
  for (size_t i = 0; i < len; ++i)
  {
    putch(str[i]);
  }
  return len;
  //return 0;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  //yield();
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) {
    *(char*)buf = '\0';
    return 0;
  }

    switch (ev.keycode)
    {
    case AM_KEY_F1:shift_pcb(1); return 0;
    case AM_KEY_F2:shift_pcb(2); return 0;
    case AM_KEY_F3:shift_pcb(3); return 0;
    default: break;
  }
  int ret = snprintf(buf, len, "%s %s\n", ev.keydown?"kd":"ku", keyname[ev.keycode]);
  printf("%s\n", buf);
  return ret;
  //return 0;

}


size_t dispinfo_read(void *buf, size_t offset, size_t len) {
    AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);

    int written = snprintf((char *)buf, len, "WIDTH:%d\nHEIGHT:%d\n", cfg.width, cfg.height);
    if (written < 0) {
      printf("dispinfo_read里written<0\n");
        return 0;
    }

    if ((size_t)written > len) {
      printf("dispinfo_read里written>len\n");
        written = len;
    }

    return (size_t)written;
}

size_t fb_write(const void *buf, size_t offset, size_t len)
{
  //yield();
  offset/=4;
  int screen_width=io_read(AM_GPU_CONFIG).width;
  io_write(AM_GPU_FBDRAW,offset % screen_width,offset / screen_width,(void*)buf,len/4,1,true);
  return len;
}
void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
