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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

# include "sem.h"

# include <conio.h>
# include <tchar.h>
# include <stdarg.h>
# include <sys/types.h>
# include <sys/time.h>
# include <fcntl.h>
# include <limits.h>

/*
 * _sem_open:
 * @name: name of the process shared semaphore. Prefix name with Global/ 
 * or Local/. 
 * @oflag:
 *
 * Posix format wrapper for opening a Windows' named semaphore
 */
_sem_t *_sem_open(const char *name, int oflag, ...){
    va_list var_args;
    int mode;
    unsigned int value;
    TRACE("_sem_open(%s)\n", name);
    // sem_open() is for named seamphores, hence:
    if (!name) {
    	errno = EINVAL;
    	DBG("_sem_open(): %s\n", strerror(errno));
    	return (_sem_t *)SEM_FAILED;
    }

    HANDLE handle=NULL;
    HANDLE controller=NULL;
    char *ctl_name = (char *)malloc(strlen(name)+strlen("-ctl")+1);
    if (!ctl_name) {
        errno = ENOMEM;
        return (_sem_t *)SEM_FAILED;
    }
    sprintf(ctl_name, "%s-ctl", name);
    if (oflag & O_CREAT) {
        va_start(var_args, oflag);
        mode = va_arg(var_args, int);
        value = va_arg(var_args, unsigned int);
        if (value > SEM_VALUE_MAX) {
    		errno = EINVAL;
           	free(ctl_name);
    		DBG("_sem_open(): %s\n", strerror(errno));
    		return (_sem_t *)SEM_FAILED;
	}
        va_end(var_args);
        handle = CreateSemaphore(NULL, (LONG) value, (LONG) SEM_VALUE_MAX, name);
        if (!handle){
           DBG("_sem_open create(%s): windows error %ld\n", name, (long)GetLastError());
           free(ctl_name);
           return (_sem_t *)SEM_FAILED;
        } 
	int last_error = GetLastError();
        if ((oflag & O_EXCL) && last_error == ERROR_ALREADY_EXISTS){
            errno = EEXIST;
            DBG("_sem_open(): %s\n", strerror(EEXIST));
            CloseHandle(handle);
            free(ctl_name);
            return (_sem_t *)SEM_FAILED;
        }
	TRACE("sem_open(): creating %s\n", ctl_name);
        controller = CreateSemaphore(NULL, (LONG) value+1, (LONG) SEM_VALUE_MAX, ctl_name);
        if (!controller){	    
            DBG("sem_open() controller %s error %ld\n", ctl_name, (long)GetLastError());
            CloseHandle(handle);
            free(ctl_name);
            return (_sem_t *)SEM_FAILED;
        }
    } else {
        handle = OpenSemaphore( SEMAPHORE_ALL_ACCESS, FALSE, name);
        if (!handle){
           DBG("_sem_open() open %s windows error %ld\n", name, (long)GetLastError());
           free(ctl_name);
           return (_sem_t *)SEM_FAILED;
        } 
	TRACE("sem_open(): opened %s->%p\n", name, handle);
        controller = OpenSemaphore( SEMAPHORE_ALL_ACCESS, FALSE, ctl_name);
        if (!controller){
           DBG("sem_open(): controller %s windows error %ld\n", ctl_name, (long)GetLastError());
           CloseHandle(handle);
           free(ctl_name);
           return (_sem_t *)SEM_FAILED;
        } 
    }

    _sem_t *_sem = (_sem_t *)malloc(sizeof(_sem_t));
    if (!_sem) {
        errno = ENOMEM;
        DBG( "_sem_open() malloc(): %s\n", strerror(errno));
        CloseHandle(handle);
        CloseHandle(controller);
        free(ctl_name);
        return (_sem_t *)SEM_FAILED;
    }
    _sem->handle = handle;
    _sem->controller = controller;
    _sem->named = 1;
    TRACE ("_sem_open() %s semaphore %s (%p) %s (%p) OK!\n",
                    (oflag & O_CREAT)?"Create":"Open",
		    name, handle,
		    ctl_name, controller); 
    free(ctl_name);
    return _sem;
}


int _sem_init(_sem_t *_sem, int pshared, unsigned int value){
    if (!_sem ||  value > (unsigned int)SEM_VALUE_MAX){
      errno = EINVAL;
      return -1;
    }
    if (pshared){
      errno = EPERM;
      return -1;
    }
    HANDLE handle = CreateSemaphore(NULL, (LONG) value, (LONG) SEM_VALUE_MAX, NULL);
    if (handle == NULL) {
            DBG( "sem_init(): CreateSemaphore(), windows error %ld\n", (long)GetLastError());
            errno = EINVAL; 
            return -1;
    }
    TRACE("--> sem_init(unnamed) got %p initialized to %d\n", handle, value);

    HANDLE controller = CreateSemaphore(NULL, (LONG) value+1, (LONG) SEM_VALUE_MAX, NULL);
    if (controller == NULL) {
            DBG( "sem_init(): CreateSemaphore(), windows error %ld\n", (long)GetLastError());
            errno = EINVAL; 
            return -1;
    }
    
    _sem->controller = controller;
    _sem->handle = handle;
    _sem->named = 0;
        
    return 0;
}

int _sem_close(_sem_t *_sem){
    TRACE("closing semaphore %p\n", _sem->handle);
    if (!_sem || _sem == SEM_FAILED) {
            errno = EBADF;
            DBG( "_sem_close(): %s\n", strerror(errno));
            return -1;
    }


    BOOL result = CloseHandle(_sem->handle);
    if (!result){
        DBG( "_sem_close(), windows error %ld\n", (long)GetLastError());
        errno = EINVAL;   
        return -1;
    }

    // If it is a named semaphore, we free allocated memory.
    if (_sem->named) {
        TRACE("Free sem %p now\n", _sem->handle);
        free(_sem);
    } else {
         //Allocated memory from sem structure comes from the calling program.
    }
    return 0;
}

int _sem_unlink(const char *name){
    // Windows unlinks when no process has the semaphore open.
    // This behavior is race prone, so beware. 
    return 0;
}

int _sem_destroy(_sem_t *_sem){
    //do nothing here.
    return 0;
}


int _sem_post_multiple(_sem_t *_sem, int count){
   TRACE("Releasing semaphore %p, %d time%s.\n", 
                   _sem->handle, count, (count > 1)?"s":"");

   ReleaseSemaphore(_sem->controller, count, NULL);
   BOOL result = ReleaseSemaphore( _sem->handle, count, NULL);
   if (!result){
        DBG( "_sem_post_multiple(), windows error %ld\n",   (long)GetLastError());
        return -1;
   }
   return 0;
}

int _sem_post(_sem_t *_sem){
   return _sem_post_multiple(_sem, 1);
}

static
int _sem_timeout(_sem_t *_sem, DWORD timeout){
   TRACE("Wait on semaphore %p\n", _sem->handle);
   DWORD result = WaitForSingleObject(_sem->handle, timeout);
   WaitForSingleObject(_sem->controller, INFINITE);
   switch (result){
      case WAIT_OBJECT_0:
          TRACE("Wait OK on semaphore %p\n", _sem->handle);
          return 0;
      case WAIT_TIMEOUT:
	  errno = ETIMEDOUT;
          TRACE("Wait timed out on semaphore %p\n", _sem->handle);
          return -1;
      case WAIT_FAILED:
	  errno = EINTR;
          TRACE("Wait failed on semaphore %p\n", _sem->handle);
          DBG( "_sem_timeout(), windows error %ld\n",  (long)GetLastError());
          return -1;
   }
   errno=EINVAL;
   return -1;
}

int _sem_trywait(_sem_t *_sem){
	if (_sem_timeout(_sem, 0) < 0){
		errno = EAGAIN;
		return -1;
	}
	return 0;
}

int _sem_wait(_sem_t *_sem){
	return _sem_timeout(_sem, INFINITE);
}
// Note: sem_timedwait is only accurate to the nearest microsecond.
int _sem_timedwait(_sem_t *_sem, const struct timespec *abs_timeout){
    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0){
           DBG( "gettimeofday(): %s\n", strerror(errno));
           return -1;
    }
    // Seconds time difference
    time_t seconds = abs_timeout->tv_sec - tv.tv_sec;
    if (seconds >= 0){
	// round off nanoseconds to nearest microsecond
    	DWORD useconds = (abs_timeout->tv_nsec + 500) / 1000;
	// Microsecond time difference (may be negative)
	useconds -= tv.tv_usec; 
	// Total microsecond difference
	useconds += (seconds * 1000);
	if (useconds >= 0) {
    		return _sem_timeout(_sem, useconds);
	}
    }
    // Timeout has expired.
    if (_sem_trywait(_sem) < 0){
	    // Cannot immediately decrease semaphore
	    errno = EAGAIN;
	    return -1;
    }
    // Semaphore immediately decreased.
    return 0;
}


// Note: usage of this function requires additional control by part
//       of the calling program (mutexes, barriers,...)
int _sem_getvalue(_sem_t * _sem, int * sval){
    if (!_sem) {
        errno = EINVAL;
        DBG( "sem_getvalue(): %s\n", strerror(errno));
        return -1;
    }
    LONG value;
    // interprocess control here
    WaitForSingleObject(_sem->controller, INFINITE);
    ReleaseSemaphore(_sem->controller, 1, &value);

    *sval = value;
    return 0;
}

//////////////////////////////////////////////////////////////////////
#ifdef PATCHED_SEM

int sem_init(sem_t *sem, int pshared, unsigned int value){
	return _sem_init(sem, pshared, value);
}

int sem_wait(sem_t *sem){
	return _sem_wait(sem);
}

int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout){
	return _sem_timedwait(sem, abs_timeout);
}

int sem_post(sem_t *sem){
	return _sem_post(sem);
}

sem_t * sem_open(const char *name, int oflag, ...){
        va_list var_args;
        int mode;
        unsigned int value;
	int long_form=0;
        va_start(var_args, oflag);
	long_form=1;
        mode = va_arg(var_args, int);
        value = va_arg(var_args, unsigned int);
        va_end(var_args);
	if (long_form){
		return _sem_open(name, oflag, mode, value);
	} 
	return _sem_open(name, oflag);
}

int sem_close(sem_t *sem){
	return _sem_close(sem);
}

int sem_unlink(const char *name){
	return _sem_unlink(name);
}

int sem_destroy(sem_t *sem){
	return _sem_destroy(sem);
}

int sem_post_multiple(sem_t *sem, int count){
	return _sem_post_multiple(sem, count);
}

int sem_getvalue(sem_t * sem, int * sval){
	return _sem_getvalue(sem, sval);
}
#endif

