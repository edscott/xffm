/*
   Semaphore wrappers Copyright (c) 2013 <edscott.wilson.garcia@gmail.com>

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/



#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H
# define  _WIN32_WINNT 0x502
# include "debug.h"
# include <windows.h>

# include <sys/stat.h>
# include <sys/time.h>
# include <semaphore.h>

# include <stdio.h>
# include <errno.h>




#ifdef __cplusplus
extern "C" {
#endif

#ifndef _TIMESPEC_DEFINED
#define _TIMESPEC_DEFINED
struct timespec {
  time_t  tv_sec;   /* Seconds */
  long    tv_nsec;  /* Nanoseconds */
};

struct itimerspec {
  struct timespec  it_interval;  /* Timer period */
  struct timespec  it_value;     /* Timer expiration */
};
#endif

#ifdef PATCHED_SEM
#define _sem_t sem_t
#else
typedef struct _sem_t {
   HANDLE handle;
   BOOL named;
   HANDLE controller;  
} _sem_t;
#endif



#ifndef SEM_FAILED
#define SEM_FAILED NULL
#endif
#ifndef SEM_VALUE_MAX
#define SEM_VALUE_MAX INT_MAX
#endif

#ifndef ETIMEDOUT
#define ETIMEDOUT 138
#endif


int _sem_init(_sem_t *sem, int pshared, unsigned int value);

int _sem_wait(_sem_t *sem);

int _sem_timedwait(_sem_t *sem, const struct timespec *abs_timeout);

int _sem_post(_sem_t *sem);

_sem_t * _sem_open(const char *name, int oflag, ...);

int _sem_close(_sem_t *sem);

int _sem_unlink(const char *name);

int _sem_destroy(_sem_t *sem);

int _sem_post_multiple(_sem_t *sem, int count);

int _sem_getvalue(_sem_t * _sem, int * sval);
       
#ifdef __cplusplus
}
#endif

#endif /* _SEMAPHORE_H */



