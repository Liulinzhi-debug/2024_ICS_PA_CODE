#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

static void sh_handle_cmd(const char *cmd) {
    size_t cmd_len = strlen(cmd);
    char cmd_buf[1024];
    if (cmd_len >= sizeof(cmd_buf)) {
        sh_printf("sh: command too long\n");
        return;
    }
    strncpy(cmd_buf, cmd, sizeof(cmd_buf) - 1);
    cmd_buf[sizeof(cmd_buf) - 1] = '\0';
    char *argv[20] = {0}; 
    int argc = 0;
    char *tok0 = strtok(cmd_buf, " \n");
    if (tok0 == NULL) return;
    argv[argc++] = tok0;
    while ((tok0 = strtok(NULL, " \n")) != NULL && argc < (int)(sizeof(argv)/sizeof(argv[0])) - 1) {
        argv[argc++] = tok0;
    }
    argv[argc] = NULL; 
   //setenv("PATH", "/bin", 0);
    if (execvp(argv[0], argv) == -1) {
        sh_printf("Error executing command: %s\n", argv[0]);
    }

}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();
  setenv("PATH", "/usr/bin:/bin", 0);
  
  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
