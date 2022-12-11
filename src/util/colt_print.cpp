#include "colt_print.h"
#include <cstdio>
#ifndef COLT_WINDOWS
#include <termios.h>
#include <unistd.h>
#else
#include <conio.h>
#endif //COLT_WINDOWS

namespace colt::io
{
  void PressToContinue() noexcept
  {
    fputs("Press any key to continue...", stdout);
#ifndef COLT_WINDOWS
    struct termios oldattr, newattr;
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    (void)getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);    
#else
    (void)_getch();
#endif //!COLT_WINDOWS
    fputc('\n', stdout);
  }
}