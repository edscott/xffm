#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif 

#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#ifdef HAVE_PROCESS_H
#include <process.h>
#endif

#ifdef HAVE_CONIO_H
#include <conio.h>
#endif

#ifdef HAVE_TCHAR_H
#include <tchar.h>
#endif

int tubo_main(int argc, char **argv);
int main(int argc, char **argv){
    return tubo_main(argc, argv);
}
