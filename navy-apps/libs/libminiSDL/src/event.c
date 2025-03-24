#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,
#define NR_KEYS 83

static uint8_t keysnap[NR_KEYS] = {0};
static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *event) {
  //assert(0);
  return 0;
}

int SDL_PollEvent(SDL_Event *event) {
  char buf[64];
  char type[10], key_name[10];
  int keycode;

  if (NDL_PollEvent(buf, 0) == 0) {
    return 0;
  }
    sscanf(buf, "%s%s\n", type, key_name);
    //printf("keycode是%d\n",keycode);
    event->type = buf[1] == 'u' ? SDL_KEYUP : SDL_KEYDOWN;
    // if(key_name=="j"||key_name=="k"||key_name=="UP"||key_name=="DOWN")
    for (int i = 0; i < sizeof(keyname) / sizeof(char*); ++ i) {
      if (strcmp(key_name, keyname[i]) == 0) {
        keycode = i;
        keysnap[i] = (event->type == SDL_KEYDOWN)? 1: 0;
        //printf("keycode是%d\n",keycode);
        break;
      }
    }

    event->key.keysym.sym = keycode;
    //printf("event->type是%d,event->key.keysym.sym是%d\n",event->type,event->key.keysym.sym);
    return 1; 

}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[64];
  char type[10], key_name[10];
  int keycode;
  while (1) {
    if (NDL_PollEvent(buf, sizeof(buf)) == 0) {
      continue;
    }
    //printf("buf=%s\n",buf);
    sscanf(buf, "%s%s\n", type, key_name);
    //printf("keycode是%d\n",keycode);
    event->type = buf[1] == 'u' ? SDL_KEYUP : SDL_KEYDOWN;
    // if(key_name=="j"||key_name=="k"||key_name=="UP"||key_name=="DOWN")
    for (int i = 0; i < sizeof(keyname) / sizeof(char*); ++ i) {
      if (strcmp(key_name, keyname[i]) == 0) {
        keycode = i;
        keysnap[i] = (event->type == SDL_KEYDOWN)? 1: 0;
        //printf("keycode是%d\n",keycode);
        break;
      }
    }

    event->key.keysym.sym = keycode;
    //printf("event->type是%d,event->key.keysym.sym是%d\n",event->type,event->key.keysym.sym);
    return 1;
  }
  return 0;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  //return NULL;
  return keysnap;
}