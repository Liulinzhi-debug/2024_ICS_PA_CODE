#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
   int i;
   AM_GPU_CONFIG_T info = io_read(AM_GPU_CONFIG);
 
   int w = info.width;  // TODO: get the correct width
   int h = info.height;  // TODO: get the correct height
   uint32_t *fb = (uint32_t *)FB_ADDR;
   for (i = 0; i < w * h; i ++) fb[i] = i;
   outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
uint32_t sc_core = inl(VGACTL_ADDR);
uint32_t sc_width = sc_core >> 16;
uint32_t sc_height = sc_core & 0xffff;
uint32_t sc_size = sc_width * sc_height * sizeof(uint32_t);

*cfg = (AM_GPU_CONFIG_T) {
  .present = true, .has_accel = false,
  .width = sc_width, .height = sc_height,
  .vmemsz = sc_size
};
  
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) 
{
  int x,y,w,h;
  x=ctl->x;
  y=ctl->y;
  w=ctl->w;
  h=ctl->h;
   if(!ctl->sync && w == 0 && h == 0)
    return;
  uint32_t *pixels=ctl->pixels;
  uint32_t sc_core = inl(VGACTL_ADDR);
  uint32_t sc_width = sc_core >> 16;
  uint32_t *fb =(uint32_t *)FB_ADDR;
   
 for(int i=y;i<y+h;i++)
 {
  for(int j=x;j<x+w;j++)
  {
    fb[sc_width*i+j]=pixels[w*(i-y)+(j-x)];
  }
 }

  if (ctl->sync) 
  {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
