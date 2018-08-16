
#ifdef COPYRIGHT_INFORMATION
#include "gplv3.h"
#endif

/*     */

/*  A program independent fork/spawn module.
 *  
 *  Copyright 2000-2013(C)  Edscott Wilson Garcia under GNU GPLv3
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; 
*/
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/time.h>

/* signal handling */
#include <signal.h>

#define H(x)
#define XH(x)

# include <sys/wait.h>

# include <sys/types.h>

# include <sys/mman.h>

#include <sys/stat.h>
#include <fcntl.h>


#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

# include <semaphore.h>
# define _sem_t sem_t
# define _sem_open(...) sem_open(__VA_ARGS__)
# define _sem_close(X)  sem_close(X)
# define _sem_unlink(X) sem_unlink(X)
# define _sem_destroy(X) sem_destroy(X)

# define _sem_getvalue(X,Y)   sem_getvalue(X,Y)
# define _sem_post(X)   sem_post(X)
# define _sem_wait(X)   sem_wait(X)
# define _sem_init(X,Y,Z) sem_init(X,Y,Z)
# define _sem_trywait(X)   sem_trywait(X)
# define _sem_timedwait(X)   sem_timedwait(X)




#include <process.h>

#ifndef _O_BINARY
#define _O_BINARY 0x0
#endif

/* public stuff */
#include "tubo.h"
#include "debug.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define TUBO_STDOUT     0x10
#define TUBO_STDERR     0x20
#define TUBO_STDIN      0x40

static int instance = 0;
static pid_t grandchildPID;

/* private stuff */


//////////////////////////////////////////////////////////

typedef struct fork_struct {
    pid_t *gchild_p;

    pid_t PID;
    pid_t grandchild;
    pid_t parent;

    char **argv;
    int tubo[3][2];
    void (*fork_function) (void *);
    void *fork_function_data;
    int stdin_fd;
    FILE *nullfd;
    void (*stdout_f) (void *stdout_data, void *stream, int childFD);
    void (*stderr_f) (void *stderr_data, void *stream, int childFD);
    void (*tubo_done_f) (void *);
    void *user_function_data;
    unsigned tubo_id;

#ifdef USE_LOCAL_SEMAPHORES
// unnamed semaphores
    _sem_t *local_semaphore;
#else
    pthread_mutex_t stdin_mutex;
    pthread_cond_t stdin_signal;  
    int  stdin_clear;
    pthread_mutex_t stdout_mutex;
    pthread_cond_t stdout_signal;  
    int  stdout_clear;
    pthread_mutex_t stderr_mutex;
    pthread_cond_t stderr_signal;   
    int  stderr_clear;
#endif

    pthread_mutex_t done_mutex;
    pthread_cond_t done_signal;  
    int  done_value;

//  named semaphores:
    // _sem_t *stdin_sem; // setup sem controls stdin
    _sem_t *stdout_sem;
    _sem_t *stderr_sem;
    _sem_t *setup_sem;


    char stdout_sem_name[64];
    char stderr_sem_name[64];
    char setup_sem_name[64];
    char shm_gchild_name[64];
    int flags;
} fork_struct;
///////////////////////////////////////////////////
typedef struct tubo_public_t{
    int id;
    pid_t PID;
    pid_t grandchild;
    struct tubo_public_t *next;
} tubo_public_t;


static tubo_public_t *tubo_public_head = NULL;
static pthread_mutex_t  list_mutex = PTHREAD_MUTEX_INITIALIZER;

static void
tubo_public_insert(pid_t PID, pid_t grandchild, int instance_in){
    pthread_mutex_lock(&list_mutex);
    if (!tubo_public_head) {
        tubo_public_head = (tubo_public_t *)malloc(sizeof(tubo_public_t));
        if (!tubo_public_head) {
            DBG( "*** Tubo: malloc(%lu): %s\n", 
                    (long unsigned) sizeof(tubo_public_t), strerror(errno));
            goto done;
        }
        tubo_public_head->PID = PID;
        tubo_public_head->grandchild = grandchild;
        tubo_public_head->next = NULL;
        goto done;
    }
    tubo_public_t *p = tubo_public_head;
    while (p->next) p = p->next;
    p->next = (tubo_public_t *)malloc(sizeof(tubo_public_t));
    p->next->PID = PID;
    p->next->grandchild = grandchild;
    p->next->id = instance_in;
    p->next->next = NULL;
done:
    pthread_mutex_unlock(&list_mutex);
    return;
}

static int
tubo_get_instance(pid_t PID){
    int id = 0;
    pthread_mutex_lock(&list_mutex);
    tubo_public_t *p = tubo_public_head;
    while (p){
        if (PID == p->PID || PID == p->grandchild){
            id = p->id;
            goto done;
        }
        p = p->next;
    }
done:
    pthread_mutex_unlock(&list_mutex);
    return id;
}


////////   remote semaphore section. /////////




static char *
_tubo_shm_name(int parent, int instance_in){
# define HEAD_SHM "/Tubo" 
    int len = strlen("%s%s-%u-%d") + 6 + 6 +  strlen(HEAD_SHM);
    char *shm_name = (char *) malloc (len+1);
    if (!shm_name){
	DBG( "*** Tubo: malloc: %s\n", strerror(errno));
	return NULL;
    }
    snprintf(shm_name, len, "%s-%u-%d",  HEAD_SHM, parent, instance_in);
    return shm_name;
}

static int
set_shm_name(char *target, const char *tag, int parent, int instance_in){
    char *shm_name = _tubo_shm_name(parent, instance_in);
    if (!shm_name) return 0;
    if (strlen(shm_name) + strlen(tag) >= 64){
        DBG( "*** Tubo: shm_name too long: %s\n", shm_name);
        free(shm_name);
        return 0;
    }
    strcpy(target, shm_name);
    strcat(target, tag);
    free(shm_name);
    return 1;
}
 

// 16 K, size of a memory page in  silicon graphics- cray boxes.
// (just a bit of ancient history)
#define TUBO_BLK_SIZE 128*128




static int
valid_ansi_sequence(char *line){
    static 
    char *invalid_sequence_p[] = { "(", ")", "M", "E", "7", "8", "H", "[A", "[g", "[0g", "[3g", "#3", "#4", "#5", "#6", "5n", "6n", "[c", NULL };
    char *esc=line;

    while (esc != NULL && *esc != 0) {
        esc=strchr(esc,0x1b);
        if (esc==NULL || *esc==0) return 1;
        esc++;
        char **p;
        for(p = invalid_sequence_p; p && *p; p++) {
            if (strncmp(esc,*p,strlen(*p))==0){
		DBG( "*** Tubo: sequence <ESC>%s is not in valid_ansi_sequence list\n", *p);
		return 0;
            }
        }
    }
    return 1;
}


#ifdef HAVE_KILL
static void 
signalit (int sig) {
 
    TRACE(" got signal %d (SIGUSR1=%d,SIGUSR2=%d)\n", sig, SIGUSR1, SIGUSR2);

    if(sig == SIGUSR1)
        kill (grandchildPID, SIGTERM);
    if(sig == SIGUSR2)
        kill (grandchildPID, SIGKILL);
    return;

}

static void
signal_connections (void ) {
//    signal(SIGHUP, signalit);
//    signal (SIGTERM, signalit);
    signal (SIGUSR1, signalit);
    signal (SIGUSR2, signalit);
}

static fork_struct *
TuboClosePipes (fork_struct * forkO) {
    int i;
    TRACE (" ClosePipes()\n");
    if(!forkO) {
        return NULL;
    }
    for(i = 0; i < 3; i++) {
        if(forkO->tubo[i][0] > 0) {
            close (forkO->tubo[i][0]);
            forkO->tubo[i][0] = -1;
        }
        if(forkO->tubo[i][1] > 0) {
            close (forkO->tubo[i][1]);
            forkO->tubo[i][1] = -1;
        }
    }
    return NULL;
}

static int
read_fd (int which, void * data) {
    static pthread_mutex_t user_f_mutex = PTHREAD_MUTEX_INITIALIZER;
    fork_struct *fork_p = (fork_struct *) data;
    char line[TUBO_BLK_SIZE];
    memset (line, 0, TUBO_BLK_SIZE);
    if(which < 1 || which > 2){
        DBG( "*** Tubo:  read_fd(): argument out of range\n");
        return 0;
    }
    int i;

    for(i = 0; i < TUBO_BLK_SIZE - 2; i++) {
        // memset (line, 0, TUBO_BLK_SIZE); above will null terminate the string.
        // coverity[string_null_argument : FALSE]
        int result = read (fork_p->tubo[which][0], line + i, 1);
        if(result < 0) {
            DBG( "*** Tubo:  read_fd(%d->%d) %s\n", which, fork_p->tubo[which][0], strerror (errno));
            return 0;
        }
        if(result == 0) {
            return 0;
        }
        if(*(line + i) == '\n') {
            *(line + i + 1) = (char)0;
            break;
        }
    }

// check for valid output (if so configured)
    if (fork_p->flags & TUBO_VALID_ANSI) {
	if (!valid_ansi_sequence(line)){
	    DBG( "*** Tubo: Sending SIGTERM to child process (flags & TUBO_REAP_CHILD==TRUE)\n");
	    // send child TERM signal: SIGUSR1 gets 
	    // translated to SIGTERM for grandchild
	    kill (fork_p->grandchild, SIGTERM); 	    
	    // ignore data
	    return 1;
	}
    }

    pthread_mutex_lock(&user_f_mutex);

    if(which == 2 && fork_p->stderr_f) {
        (*(fork_p->stderr_f)) (fork_p->user_function_data, (void *)line, fork_p->tubo[0][1]);
    } else if(which == 1 && fork_p->stdout_f) {
        (*(fork_p->stdout_f)) (fork_p->user_function_data, (void *)line, fork_p->tubo[0][1]);
    }
    pthread_mutex_unlock(&user_f_mutex);
    return 1;

}

static void
stdio_f (int which, void * data) {
    fork_struct *fork_p = (fork_struct *) data;

    TRACE (" stdio(%d) thread setup!\n", which);
    //map_remote_semaphores(fork_p);
    // greenlight to exec:
    //
    // These semaphores are now in separate
    // memory blocks, so mutex is not necessary
    // any more.
    if (which ==1) {
        TRACE("****: thread 0x%x posting semaphore %s\n", getpid(), fork_p->stdout_sem_name);
	//{int v;_sem_getvalue(fork_p->stdout_sem, &v);fprintf(stderr,"sem %s before post, value=%d\n", fork_p->stdout_sem_name,v);fflush(stderr);} 
        _sem_post(fork_p->stdout_sem);
	//{int v;_sem_getvalue(fork_p->stdout_sem, &v);fprintf(stderr,"sem %s after post, value=%d\n", fork_p->stdout_sem_name,v);fflush(stderr);} 

        // This is racy in Windows, since close implies unlink as well.
        // So in windows we defer close later on.
        _sem_close(fork_p->stdout_sem);

    } else {
        TRACE("****: thread 0x%x posting semaphore %s\n", getpid(), fork_p->stderr_sem_name);
	//{int v;_sem_getvalue(fork_p->stdout_sem, &v);fprintf(stderr,"sem %s before post, value=%d\n", fork_p->stderr_sem_name,v);fflush(stderr);} 
        _sem_post(fork_p->stderr_sem);
	//{int v;_sem_getvalue(fork_p->stdout_sem, &v);fprintf(stderr,"sem %s after post, value=%d\n", fork_p->stderr_sem_name,v);fflush(stderr);} 
        // This is racy in Windows, since close implies unlink as well.
        // So in windows we defer close later on.
        _sem_close(fork_p->stderr_sem);
    }

    do {
        if (!(read_fd (which, (void *)fork_p))) break;
    } while (1);
    //while(read_fd (which, (void *)fork_p)) ;

    TRACE( " closing fd(%d)-> %d\n", which, fork_p->tubo[which][0]);
    close (fork_p->tubo[which][0]);
    // greenlight to fork_finished function:
#ifdef USE_LOCAL_SEMAPHORES
    TRACE( "stdio_f posting local semaphore %d (%p)\n", which, (void *) (fork_p->local_semaphore + which));
	//{int v;_sem_getvalue(fork_p->local_semaphore + which, &v);fprintf(stderr,"sem %p before post, value=%d\n", (void *)(fork_p->local_semaphore + which),v);fflush(stderr);} 
    _sem_post (fork_p->local_semaphore + which);
	//{int v;_sem_getvalue(fork_p->local_semaphore + which, &v);fprintf(stderr,"sem %p after post, value=%d\n", (void *)(fork_p->local_semaphore),v);fflush(stderr);} 
#else
    TRACE( " stdio_f signal to local %s\n", (which==1)?"stdout_signal":"stderr_signal");
    if (which == 1) {
            pthread_mutex_lock(&(fork_p->stdout_mutex));
            fork_p->stdout_clear = 1;
            pthread_mutex_unlock(&(fork_p->stdout_mutex)); 
            pthread_cond_signal(&(fork_p->stdout_signal)); 
    } else {
            pthread_mutex_lock(&(fork_p->stderr_mutex)); 
            fork_p->stderr_clear = 1;
            pthread_mutex_unlock(&(fork_p->stderr_mutex)); 
            pthread_cond_signal(&(fork_p->stderr_signal)); 
    }

    pthread_mutex_lock(&(fork_p->done_mutex));
    fork_p->done_value++;
    pthread_mutex_unlock(&(fork_p->done_mutex)); 
    pthread_cond_signal(&(fork_p->done_signal)); 


#endif    
    TRACE( "Sem is now posted.\n");
}

static void *
stdout_thread_f (void * data) {
    stdio_f (1, data);
    TRACE( "oooo  stdout_thread_f is now complete...\n");
    return NULL;
}

static void *
stderr_thread_f (void * data) {
    stdio_f (2, data);
    TRACE( "oooo  stderr_thread_f is now complete...\n");
    return NULL;
}

static void *
threaded_wait_f (void * data) {
    fork_struct *fork_p = (fork_struct *) data;
    int status;
    //map_remote_semaphores(fork_p);
    TRACE ("....  thread=0x%x, wait for 0x%x\n", (unsigned)getpid (), (unsigned)(fork_p->PID));

    if(fork_p->flags & TUBO_REAP_CHILD) {
        waitpid (fork_p->PID, &status, 0);
    } 
    else {
        // leave child in waitable state...
        // no such thing in BSD
        siginfo_t infop;
        waitid (P_PID, fork_p->PID, &infop, WNOWAIT);
    }

#ifdef USE_LOCAL_SEMAPHORES
    // TRACE (" threaded_wait_f waiting for semaphore 0 (stdin)...\n");
    // _sem_wait (fork_p->local_semaphore); // go ahead with stdin
    // _sem_close(fork_p->local_semaphore);
    if (fork_p->stderr_f) {
        TRACE (" threaded_wait_f waiting for local semaphore 2 (stderr)...\n");
	//{int v;_sem_getvalue((fork_p->local_semaphore + 2), &v);fprintf(stderr,"sem %p before wait, value=%d\n", (void *)(fork_p->local_semaphore + 2),v);fflush(stderr);} 
        _sem_wait (fork_p->local_semaphore + 2);
	//{int v;_sem_getvalue((fork_p->local_semaphore + 2), &v);fprintf(stderr,"sem %p after wait, value=%d\n", (void *)(fork_p->local_semaphore + 2),v);fflush(stderr);} 
        _sem_close(fork_p->local_semaphore + 2);
        _sem_destroy(fork_p->local_semaphore + 2);
    }
    if (fork_p->stdout_f){
        TRACE (" threaded_wait_f waiting for local semaphore 1 (stdout)...\n");
	//{int v;_sem_getvalue((fork_p->local_semaphore + 1), &v);fprintf(stderr,"sem %p before wait, value=%d\n", (void *)(fork_p->local_semaphore + 1),v);fflush(stderr);} 
        _sem_wait (fork_p->local_semaphore + 1);    
	//{int v;_sem_getvalue((fork_p->local_semaphore + 1), &v);fprintf(stderr,"sem %p after wait, value=%d\n", (void *)(fork_p->local_semaphore + 1),v);fflush(stderr);} 
        _sem_close(fork_p->local_semaphore + 1);
        _sem_destroy(fork_p->local_semaphore + 2);
    }
    TRACE( " semaphores OK!\n");
    free (fork_p->local_semaphore);
#else
    TRACE (" threaded_wait_f waiting for condition 1...\n");
    pthread_mutex_lock(&(fork_p->stdout_mutex));
    if (!fork_p->stdout_clear){
        pthread_cond_wait(&(fork_p->stdout_signal),&(fork_p->stdout_mutex)); 
    }
    pthread_mutex_unlock(&(fork_p->stdout_mutex));   
    TRACE (" threaded_wait_f waiting for condition 0...\n");
    pthread_mutex_lock(&(fork_p->stderr_mutex)); 
    if (!fork_p->stderr_clear){
        pthread_cond_wait(&(fork_p->stderr_signal),&(fork_p->stderr_mutex)); 
    }
    pthread_mutex_unlock(&(fork_p->stderr_mutex)); 
    TRACE( " conditions OK!\n");
#endif


    if(fork_p->tubo_done_f) {
        TRACE(" running tubo_done_f() now...\n");
        (*(fork_p->tubo_done_f)) (fork_p->user_function_data);
    }
    // finally, close stdin. 
    if(fork_p->tubo[0][1] > 0)
        close (fork_p->tubo[0][1]);


    TRACE( "oooo  threaded_wait_f complete for 0x%x\n", (unsigned)(fork_p->PID));

    
    pthread_mutex_lock(&(fork_p->done_mutex));
    do {
        pthread_cond_wait(&(fork_p->done_signal),&(fork_p->done_mutex)); 
    } while (fork_p->done_value < 2);
    pthread_mutex_unlock(&(fork_p->done_mutex)); 

    free (fork_p);
    return NULL;
}



static void 
controller(fork_struct *newfork, int forked, int instance_in, pid_t parent){
     TRACE("Controller starting...\n");

    // These file descriptors are not inherited with spawnvp
    // They are probably open, but they will get closed when
    // the controller _exits.
    // With fork() we cleanly close them now, if they are
    // open.
    /* stdin for write, not used: */
    if (newfork->tubo[0][1] > 0) close (newfork->tubo[0][1]);
    /* stdout for read, not used: */
    if (newfork->tubo[1][0] > 0) close (newfork->tubo[1][0]);
    /* stderr for read, not used: */
    if (newfork->tubo[2][0] > 0) close (newfork->tubo[2][0]);
    /* stdin for read: */

    // wait on stdout_sem
    if(newfork->stdout_f != NULL) {
        if (!set_shm_name(newfork->stdout_sem_name, "-stdout", parent, instance_in)) _exit(123);
        // open named sem
        newfork->stdout_sem = _sem_open(newfork->stdout_sem_name, 0, 0700, 0);
        if (newfork->stdout_sem == SEM_FAILED) {
            DBG( "*** Tubo: Cannot open named semaphore: %s (%s)\n", 
                    newfork->stdout_sem_name, strerror(errno));
        } else {
            // wait on remote sem
            TRACE("----: child wait for named semaphore %s...\n", newfork->stdout_sem_name);
	    //{int v;_sem_getvalue(newfork->stdout_sem, &v);fprintf(stderr,"sem %s before wait, value=%d\n", newfork->stdout_sem_name,v);fflush(stderr);} 
            _sem_wait(newfork->stdout_sem);
	    //{int v;_sem_getvalue(newfork->stdout_sem, &v);fprintf(stderr,"sem %s after wait, value=%d\n", newfork->stdout_sem_name,v);fflush(stderr);} 
            TRACE("++++: child got semaphore %s\n", newfork->stdout_sem_name);   
            // close remote sem
            _sem_close(newfork->stdout_sem);
            // remove remote sem (no longer in use)
            _sem_unlink(newfork->stdout_sem_name);
        }
    }
    // wait on stderr_sem
    if(newfork->stderr_f != NULL) {
        if (!set_shm_name(newfork->stderr_sem_name, "-stderr", parent, instance_in)) _exit(123);
        // open named sem
        newfork->stderr_sem = _sem_open(newfork->stderr_sem_name, 0, 0700, 0);
        if (newfork->stderr_sem == SEM_FAILED) {
            DBG( "*** Tubo: Cannot open named semaphore: %s (%s)\n", 
                    newfork->stderr_sem_name, strerror(errno));
        } else {
            // wait on remote sem
            TRACE("----: child wait for named semaphore %s...\n", newfork->stderr_sem_name);
	    //{int v;_sem_getvalue(newfork->stderr_sem, &v);fprintf(stderr,"sem %s before wait, value=%d\n", newfork->stderr_sem_name,v);fflush(stderr);} 
            _sem_wait(newfork->stderr_sem);
	    //{int v;_sem_getvalue(newfork->stderr_sem, &v);fprintf(stderr,"sem %s after wait, value=%d\n", newfork->stderr_sem_name,v);fflush(stderr);} 
            TRACE("++++: child got semaphore %s\n", newfork->stderr_sem_name);   
            // close remote sem
            _sem_close(newfork->stderr_sem);
            // remove remote sem (no longer in use)
            _sem_unlink(newfork->stderr_sem_name);
        }
     }

    if (!set_shm_name(newfork->setup_sem_name, "-setup", parent, instance_in)) _exit(123);
    newfork->setup_sem = _sem_open(newfork->setup_sem_name, 0, 0700, 0);
    if (newfork->setup_sem == SEM_FAILED) {
        DBG( "*** Tubo: Cannot open named semaphore: %s (%s)\n", 
                    newfork->setup_sem_name, strerror(errno));
    }
        
    TRACE( "grandchildPID fork... \n");
    grandchildPID = fork ();  	
    if(grandchildPID == 0) {
            TRACE("grandchild here... \n");
        if (forked) {
            TRACE( "grandchild here A... \n");
            setpgid (0, 0);     /* or setpgrp(); */
            if(newfork->fork_function)
                (*(newfork->fork_function)) (newfork->fork_function_data);
            DBG( "Tubo_thread incorrect usage: fork_function must _exit()\n");
            _exit (123);
        } else {
            TRACE( "grandchild here B... \n");
            TRACE( "* execvp %s\n", newfork->argv[0]);
            execvp(newfork->argv[0], newfork->argv);
            DBG( "*** Tubo:  Cannot execvp %s (%s)\n", newfork->argv[0],strerror(errno));
            _exit(123);
        }
    } 
	
    // put grandchild pid into shm 
    if (!set_shm_name(newfork->shm_gchild_name, "-gchild", parent, instance_in))_exit(123);
    
    int fd = 
        shm_open(newfork->shm_gchild_name, O_RDWR, 0700);
    if(fd < 0){
        DBG( "*** Tubo: child shm open(%s): %s\n", newfork->shm_gchild_name, strerror (errno));
    } else {
	    TRACE( "C>>>> grandchild id is 0x%x\n", grandchildPID);
	    pid_t *gchild = 
	     	mmap(NULL, sizeof (pid_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	    close(fd);
	    if (gchild == MAP_FAILED){
	        DBG( "*** Tubo: child mmap failed for %s: %s\n", 
			newfork->shm_gchild_name, strerror (errno));
	    } else{
		*gchild = grandchildPID;
		if(msync (gchild, sizeof(pid_t), MS_SYNC) < 0){
		    DBG( "*** Tubo: Child msync(%s): %s\n", newfork->shm_gchild_name, strerror (errno));
		}
		munmap (gchild, sizeof(pid_t));
	    }
    }    
    

    TRACE("****: child posting semaphore %s\n", newfork->setup_sem_name);
	//{int v;_sem_getvalue(newfork->stdout_sem, &v);fprintf(stderr,"sem %s before post, value=%d\n", newfork->setup_sem_name,v);fflush(stderr);} 
    if (newfork->setup_sem != SEM_FAILED){
        _sem_post(newfork->setup_sem);
            //{int v;_sem_getvalue(newfork->stdout_sem, &v);fprintf(stderr,"sem %s after post, value=%d\n", newfork->setup_sem_name,v);fflush(stderr);} 
        // It is okay to close this here (in Windows) since the handle
        // is dupped in the parent.
        _sem_close(newfork->setup_sem);
    }
    signal_connections ();  // pass signals to the grandchild (disabled in windoze).

    int status;
    // this global variable is for SIGTERM conditions
    TRACE( " child process 0x%x is waiting for grandchild 0x%x\n",
        getpid(), grandchildPID);

    waitpid (grandchildPID, &status, 0);

    TRACE( " child process wait complete\n");

    // This here is to let other side of pipe know that all pipe output
    // is hencefore done with. 
    // In previous tubo versions, fork finished function may execute
    // before all pipe output is done with.
    // This is no longer necessary but kept for backward compatibility.
    if (newfork->stdout_f) {
        if (newfork->flags & TUBO_EXIT_TEXT) {
            fprintf(stdout, "Tubo-id exit:%d> (%d)\n", 
                newfork->tubo_id, (int)(getpid ()));
        } else {
            fprintf(stdout, "\n"); 
        }
        fflush(stdout);
    }
    /*if (newfork->stderr_f) {
        fprintf(stderr, "\n"); 
        fflush(stderr);
    }    */    

    TRACE(" exit %d\n", getpid());
    free(newfork);
    TRACE( "OOOO  controller process now complete\n");


    if (forked) _exit (123);

    return;

}


static void 
parent_setup(fork_struct *newfork)    {      

        // create a condition so that finished function will
        // not start until all threads are done.

        // Local semaphores (or conditions)
#ifdef USE_LOCAL_SEMAPHORES
        TRACE("mallocing local semaphores...\n");
        newfork->local_semaphore = (_sem_t *) malloc (3 * sizeof (_sem_t));
	if (!newfork->local_semaphore){
	    DBG( "*** Tubo: malloc(%lu) %s\n", 
                    (long unsigned)(3 * sizeof (_sem_t)), strerror(errno));
	    return;
	}
        TRACE("initializing local semaphores...\n");
        //_sem_init (newfork->local_semaphore, 0, 1); // go ahead with stdin
        if (newfork->stdout_f) _sem_init (newfork->local_semaphore + 1, 0, 0);
        if (newfork->stderr_f) _sem_init (newfork->local_semaphore + 2, 0, 0);        
        TRACE("initialized local semaphores...\n");
#else
        pthread_mutex_init(&(newfork->stdin_mutex), NULL);
        pthread_cond_init (&(newfork->stdin_signal), NULL);
        newfork->stdin_clear = 1; // go ahead with stdin

        pthread_mutex_init(&(newfork->stdout_mutex), NULL);
        pthread_cond_init (&(newfork->stdout_signal), NULL);
        newfork->stdout_clear = (newfork->stdout_f == NULL)?1:0;
        
        pthread_mutex_init(&(newfork->stderr_mutex), NULL);
        pthread_cond_init (&(newfork->stderr_signal), NULL);
        newfork->stderr_clear = (newfork->stderr_f == NULL)?1:0;

#endif

        pthread_mutex_init(&(newfork->done_mutex), NULL);
        pthread_cond_init (&(newfork->done_signal), NULL);
        newfork->done_value = 0;

        TRACE (" parent=0x%x child=0x%x\n", (unsigned)getpid (), (unsigned)newfork->PID);

        /* INPUT PIPES *************** */


        /* stdin for write: */
        // This is ready now... (methinks)
        // greenlight to fork_finished function:


        /* stdout for read: */
        if(newfork->stdout_f == NULL) {
            // greenlight to fork_finished function:
            newfork->done_value++;
        } else {
            TRACE( " parent creating stdout thread\n");
	    pthread_t thread;
	    pthread_create(&thread, NULL, stdout_thread_f, (void *)newfork);
	    pthread_detach(thread);
        }
        /* stderr for read: */
        if(newfork->stderr_f == NULL) {
            // greenlight to fork_finished function:
            if (newfork->tubo[2][0]>0){
                    DBG( "*** Tubo: file descriptor 2-r should not be open\n");
            }
            newfork->done_value++;
        } else {
            TRACE (" parent creating stderr thread\n");
	    pthread_t thread;
	    pthread_create(&thread, NULL, stderr_thread_f, (void *)newfork);
	    pthread_detach(thread);
        }

       
        /* fire off a threaded wait for the child */
	pthread_t thread;
	pthread_create(&thread, NULL, threaded_wait_f, (void *)newfork);
	pthread_detach(thread);

        /* threads are now in place and ready to read from pipes,
         * child process will get green light to exec
         * as soon as stdin/stdout threads are ready... */

        TRACE( "----: parent process waiting for semaphore %s\n", newfork->setup_sem_name);
	//{int v;_sem_getvalue(newfork->setup_sem, &v);fprintf(stderr,"sem %s before wait, value=%d\n", newfork->setup_sem_name,v);fflush(stderr);} 
        _sem_wait(newfork->setup_sem);
	//{int v;_sem_getvalue(newfork->setup_sem, &v);fprintf(stderr,"sem %s after wait, value=%d\n", newfork->setup_sem_name,v);fflush(stderr);} 
        TRACE( "++++: parent process got semaphore %s\n", newfork->setup_sem_name);
        // Note: windows will unlink semaphore automatically when all referenced
        //       handles are closed. This behavior creates a potential race
        //       condition which must be taken into account.
        //       Here it is OK to remove semaphore.
        
        _sem_close(newfork->setup_sem);
        _sem_unlink(newfork->setup_sem_name);

        // If stdout and stderr are done with, we give the child
        // a green light to exit when ready. Otherwise, stdio_f
        // will send the green light in due time.

        
        // Get the grandchild pid from shared memory:
        // 
        newfork->grandchild = *(newfork->gchild_p);
        munmap (newfork->gchild_p, sizeof(pid_t));
        shm_unlink(newfork->shm_gchild_name);
        TRACE( "P>>>> grandchild id is 0x%x\n", newfork->grandchild);

        tubo_public_insert(newfork->PID, newfork->grandchild, newfork->tubo_id);
        
        TRACE( " The parent process is returning, grandchild 0x%x\n", newfork->grandchild);
        

        TRACE("  parent %d<->%d   %d<->%d   %d<->%d\n",
               newfork->tubo[0][0],
               newfork->tubo[0][1], newfork->tubo[1][0], newfork->tubo[1][1], newfork->tubo[2][0], newfork->tubo[2][1]);
        return;    
}



static int
restore_fd(int *save_p, int original){
    if (*save_p != -1) {
      if(dup2(*save_p, original) < 0){
          DBG("*** Tubo: cannot restore %d\n", original);
          close(*save_p);        
          return 0;
      }
      close(*save_p);        
    }
    return 1;
}

static int
setup_pipes_1(fork_struct *newfork, int *save_stdin_p, int *save_stdout_p, int *save_stderr_p){

    // Always save stdin, because if it is not piped, then it is set to /dev/null
    int in_fd = fileno(stdin);
    if (in_fd >= 0) *save_stdin_p = dup(in_fd);
    else *save_stdin_p = -1;

    if (newfork->stdin_fd != -1) {
        // This sets stdin straight from the pipe.
        if (dup2(newfork->tubo[0][0], fileno(stdin)) < 0){
            // unable to create pipe
            DBG( "*** Tubo: unable to create stdin pipe with dup2\n");
            restore_fd(save_stdin_p, fileno(stdin));
            return 0;
        }
        close(newfork->tubo[0][0]);
        newfork->tubo[0][0] = -1;
        // setup semaphore will control stdin...
    } else {
        // Nothing interesting in stdin, so don't find anything...
        int fd = in_fd;
        if (fd >= 0){
	    newfork->nullfd = fopen ("/dev/null", "rb");
	    if (dup2 (fileno (newfork->nullfd), fd) < 0){
                // unable to create pipe
                DBG( "*** Tubo: unable to create stdin pipe with dup2\n");
                restore_fd(save_stdin_p, fileno(stdin));

                return 0;
            }
        }
    }


    *save_stdout_p = -1;
    if (newfork->stdout_f) {
        // This sets stdout straight for the pipe.
        *save_stdout_p = dup(fileno(stdout));
        if (dup2(newfork->tubo[1][1], fileno(stdout)) < 0){
            // unable to create pipe
            restore_fd(save_stdin_p, fileno(stdin));
            restore_fd(save_stdout_p, fileno(stdout));
            DBG( "*** Tubo: unable to create stdout pipe with dup2\n");
            
            return 0;
        }
        close(newfork->tubo[1][1]);
        newfork->tubo[1][1] = -1;
        if (!set_shm_name(newfork->stdout_sem_name, "-stdout", getpid(), instance))return 0;
        newfork->stdout_sem = _sem_open(newfork->stdout_sem_name, O_CREAT|O_EXCL, 0700, 0);
        if (newfork->stdout_sem == SEM_FAILED) {
            restore_fd(save_stdin_p, fileno(stdin));
            restore_fd(save_stdout_p, fileno(stdout));
            DBG( "*** Tubo: Cannot create named semaphore: %s (%s)\n", 
                    newfork->stdout_sem_name, strerror(errno));
            return 0;
        }
        
    } 

    *save_stderr_p = -1;
    if (newfork->stderr_f) {
        // This sets stderr straight for the pipe.
        *save_stderr_p = dup(fileno(stderr));
        if (dup2(newfork->tubo[2][1], fileno(stderr)) < 0){
            // unable to create pipe
            restore_fd(save_stdin_p, fileno(stdin));
            restore_fd(save_stderr_p, fileno(stderr));
            restore_fd(save_stdout_p, fileno(stdout));
            DBG("unable to create stderr pipe with dup2\n");
            return 0;
        }
        close(newfork->tubo[2][1]);
        newfork->tubo[2][1] = -1;
        if (!set_shm_name(newfork->stderr_sem_name, "-stderr", getpid(), instance))return 0;
        newfork->stderr_sem = _sem_open(newfork->stderr_sem_name, O_CREAT|O_EXCL, 0700, 0);
        if (newfork->stderr_sem == SEM_FAILED) {
            restore_fd(save_stdin_p, fileno(stdin));
            restore_fd(save_stdout_p, fileno(stdout));
            restore_fd(save_stderr_p, fileno(stderr));
            DBG( "*** Tubo: Cannot create named semaphore: %s (%s)\n", 
                    newfork->stderr_sem_name, strerror(errno));
            return 0;
        }
    }

    if (!set_shm_name(newfork->setup_sem_name, "-setup", getpid(), instance))return 0;
    newfork->setup_sem = _sem_open(newfork->setup_sem_name, O_CREAT|O_EXCL, 0700, 0);
    if (newfork->setup_sem == SEM_FAILED) {
            restore_fd(save_stdin_p, fileno(stdin));
            restore_fd(save_stdout_p, fileno(stdout));
            restore_fd(save_stderr_p, fileno(stderr));
        DBG( "*** Tubo: Cannot create named semaphore: %s (%s)\n", 
                newfork->setup_sem_name, strerror(errno));
            return 0;
    }
    if (!set_shm_name(newfork->shm_gchild_name, "-gchild", getpid(), instance))return 0;
    int fd = shm_open(newfork->shm_gchild_name, O_RDWR | O_CREAT | O_EXCL, 0700);
    if(fd < 0){
            restore_fd(save_stdin_p, fileno(stdin));
            restore_fd(save_stdout_p, fileno(stdout));
            restore_fd(save_stderr_p, fileno(stderr));
        DBG( "*** Tubo: parent shm open(%s): %s\n", newfork->shm_gchild_name, strerror (errno));
            return 0;
    } else {
        if(ftruncate (fd, sizeof (pid_t)) < 0) {
            restore_fd(save_stdin_p, fileno(stdin));
            restore_fd(save_stdout_p, fileno(stdout));
            restore_fd(save_stderr_p, fileno(stderr));
            DBG( "*** Tubo: ftruncate(%s): %s\n", newfork->shm_gchild_name, strerror (errno));
	    close(fd);
            return 0;
        }
        newfork->gchild_p = 
	     	mmap(NULL, sizeof (pid_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	if ( newfork->gchild_p == MAP_FAILED){
            restore_fd(save_stdin_p, fileno(stdin));
            restore_fd(save_stdout_p, fileno(stdout));
            restore_fd(save_stderr_p, fileno(stderr));
	     DBG( "*** Tubo: mmap failed for %s: %s\n", 
		newfork->shm_gchild_name, strerror (errno));
            return 0;
	} 

    }

    return 1;
}


static
int setup_pipes_2(fork_struct *newfork, int save_stdin, int save_stdout, int save_stderr){
    // This function is executed by the parent after the fork or spawn
    // Now we must reset stdout and stderr to saved values.
    if (newfork->nullfd) fclose(newfork->nullfd);
    restore_fd(&save_stdin, fileno(stdin));
    restore_fd(&save_stdout, fileno(stdout));
    restore_fd(&save_stderr, fileno(stderr));
    // stdin for read, not used by parent: 
    // If pipe was not opened, the fd == -1. 
    if (newfork->tubo[0][0] > 0) {
        close (newfork->tubo[0][0]);
        TRACE (" parent closing unused fd %d\n", newfork->tubo[0][0]);
        newfork->tubo[0][0] = -1;
    }

    /* stdout for write, not used: */
    if (newfork->tubo[1][1] > 0) {
        close (newfork->tubo[1][1]);
        TRACE (" parent closing unused fd %d\n", newfork->tubo[1][1]);
        newfork->tubo[1][1] = -1;
    }

    /* stderr for write, not used: */
    if (newfork->tubo[2][1] > 0) {
        close (newfork->tubo[2][1]);
        TRACE (" parent closing unused fd %d\n", newfork->tubo[2][1]);
        newfork->tubo[2][1] = -1;
    }

    return 1;
}

#define PIPE(x) pipe(x)

static fork_struct * 
fork_struct_new(
              char **argv,
              void (*fork_function) (void *),
              void *fork_function_data,
              int *stdin_fd_p,
              void (*stdout_f) (void *stdout_data,
                                void *stream,
                                int childFD),
              void (*stderr_f) (void *stderr_data,
                                void *stream,
                                int childFD), 
	      void (*tubo_done_f) (void *), 
	      void *user_function_data, 
	      int flags
        ){
    TRACE("Parent=0x%x\n", getpid());
    fork_struct *newfork = (fork_struct *)malloc(sizeof(fork_struct));
    if (!newfork) {DBG("*** Tubo: malloc: %s\n", strerror(errno)); return NULL;}

    memset (newfork, 0, sizeof (fork_struct));

    instance++;
    newfork->flags = flags;

    int i; for(i = 0; i < 3; i++) newfork->tubo[i][0] = newfork->tubo[i][1] = -1;

    if (stdin_fd_p){
        if(PIPE(newfork->tubo[0]) == -1 || newfork->tubo[0][0] <= 2  || newfork->tubo[0][1] <= 2) {
            DBG("Incorrect pipes (0): %d <-> %d \n", newfork->tubo[0][0], newfork->tubo[0][1]); 
            TuboClosePipes (newfork);
            newfork->stdin_fd = -1;
            free(newfork);
            return 0;
        }
        *stdin_fd_p = newfork->tubo[0][1];
        newfork->stdin_fd = *stdin_fd_p;
    } else {
        newfork->stdin_fd = -1;
    }

    if (stdout_f){
        if(PIPE(newfork->tubo[1]) == -1 || newfork->tubo[1][0] <= 2  || newfork->tubo[1][1] <= 2) {
            DBG("Incorrect pipes (1): %d <-> %d \n", newfork->tubo[1][0], newfork->tubo[1][1]); 
            TuboClosePipes (newfork);
            free(newfork);
            return 0;
        }

    }

    if (stderr_f){
        if(PIPE(newfork->tubo[2]) == -1 || newfork->tubo[2][0] <= 2  || newfork->tubo[2][1] <= 2) {
            DBG("Incorrect pipes (2): %d <-> %d \n", newfork->tubo[2][0], newfork->tubo[2][1]); 
            TuboClosePipes (newfork);
            free(newfork);
            return 0;
        }

    }

    TRACE ("  before fork: %d<->%d   %d<->%d   %d<->%d\n",
           newfork->tubo[0][0], newfork->tubo[0][1], newfork->tubo[1][0], newfork->tubo[1][1], newfork->tubo[2][0], newfork->tubo[2][1]);

    newfork->argv = argv;
    newfork->fork_function = fork_function;
    newfork->fork_function_data = fork_function_data;

    
    newfork->stdout_f = stdout_f;
    newfork->stderr_f = stderr_f;
    newfork->tubo_done_f = tubo_done_f;
    newfork->user_function_data = user_function_data;

    newfork->tubo_id = instance;
    return newfork;


}

