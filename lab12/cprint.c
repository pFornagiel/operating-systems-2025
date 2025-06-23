#include "cprint.h"
#include <stdarg.h>
#include <stdio.h>

#define ANSI_COLOR_RESET  "\x1b[0m"

void cprintf(char * color, char * text, ...){  
  va_list args;
  printf("%s", color); 

  va_start(args, text);
  vprintf(text, args);  // handling variable arguments
  va_end(args);

  printf("%s", ANSI_COLOR_RESET); 
  fflush(stdout);
  
}