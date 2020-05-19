#ifndef XFTUBO_HH
#define XFTUBO_HH
#include <sys/time.h>
#include <signal.h>
# include <sys/wait.h>
# include <sys/mman.h>
# include <sys/types.h>
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
#include <pthread.h>
#include <glib.h>


#ifdef HAVE_PROCESS_H
#include <process.h>
#endif



#define TUBO_STDOUT     0x10
#define TUBO_STDERR     0x20
#define TUBO_STDIN      0x40
/**
 * TUBO_REAP_CHILD
 *
 * Flag to do reap child process on termination. If this is not set, 
 * calling program is responsible for reaping the child process.
 *
 **/
#define TUBO_REAP_CHILD 0x01

/**
 * TUBO_VALID_ANSI
 *
 * Flag to do reap child process on termination. If this is not set, 
 * calling program is responsible for reaping the child process.
 *
 **/
#define TUBO_VALID_ANSI 0x02

/**
 * TUBO_EXIT_TEXT
 *
 * Use this flag to have the controller print the standard "Tubo exit"
 * text to stdout on program termination. This is the default in
 * Tubo_threads().
 *
 **/
#define TUBO_EXIT_TEXT  0x04

/**
 * TUBO_CONTROLLER_PID
 *
 * Use this flag to have Tubo_exec() and Fork() return the controller
 * pid instead of the pid of the program being executed. This is the default
 * in Tubo_threads().
 *
 **/
#define TUBO_CONTROLLER_PID  0x08


/*
template <class TypeClass>
class Fork {
public:
    int id;
    pid_t PID;
    pid_t grandchild;
    void *next;

private:
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

// unnamed semaphores
    sem_t *local_semaphore;

    pthread_mutex_t done_mutex;
    pthread_cond_t done_signal;  
    int  done_value;

//  named semaphores:
    // sem_t *stdin_sem; // setup sem controls stdin
    sem_t *stdout_sem;
    sem_t *stderr_sem;
    sem_t *setup_sem;


    char stdout_sem_name[64];
    char stderr_sem_name[64];
    char setup_sem_name[64];
    char shm_gchild_name[64];
    int flags;
};

*/
static GSList *tuboList = NULL;
static gint instance = 0;
static pid_t grandchildPID;


typedef struct forkStruct_t {
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

// unnamed semaphores
    sem_t *local_semaphore;

    pthread_mutex_t done_mutex;
    pthread_cond_t done_signal;  
    int  done_value;

//  named semaphores:
    // sem_t *stdin_sem; // setup sem controls stdin
    sem_t *stdout_sem;
    sem_t *stderr_sem;
    sem_t *setup_sem;


    char stdout_sem_name[64];
    char stderr_sem_name[64];
    char setup_sem_name[64];
    char shm_gchild_name[64];
    int flags;

} forkStruct_t;

typedef struct tuboPublic_t{
    int id;
    pid_t PID;
    pid_t grandchild;
    struct tuboPublic_t *next;
} tuboPublic_t;

static pthread_mutex_t  list_mutex = PTHREAD_MUTEX_INITIALIZER;

namespace xf {

template <class Type>
class Tubo {
public:
    static pid_t Fork (
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
        forkStruct_t *newfork = forkStructNew(NULL, fork_function, fork_function_data,
                                stdin_fd_p, stdout_f, stderr_f, tubo_done_f,
                                user_function_data, flags);
        if (!newfork) return 0;

        gint save_stdin = -1;
        gint save_stdout = -1;
        gint save_stderr = -1;

        if (!setupPipes1(newfork, &save_stdin, &save_stdout, &save_stderr)) {
            if (save_stdin != -1) close(save_stdin);
            if (save_stdout != -1) close(save_stdout);
            if (save_stderr != -1) close(save_stderr);
            free(newfork);
            return 0;
        }

            
        pid_t parent=getpid();
            
        // The main fork
        pid_t PID = fork ();
	if (PID < 0) {
	    fprintf(stderr, "fork(): %s\n", strerror(errno));
	    return PID;
	}
        newfork->PID = PID;
        if(PID == 0) { /* the child */
            controller(newfork, 1, instance, parent);
            _exit(123);
        }

        // Now we must reset stdout and stderr to saved values.
        if (!setupPipes2(newfork, save_stdin, save_stdout, save_stderr)) return 0;

        parentSetup(newfork);
        // Child threads are still using newfork

        pid_t grandchild = newfork->grandchild;
        if (flags & TUBO_CONTROLLER_PID) return PID; 
        return grandchild;
    }

    static gint getChild(pid_t parent){
        GSList *list;
        for (list = tuboList; list && list->data; list=list->next){
            tuboPublic_t *p = (tuboPublic_t *)list->data;
            if (p->PID == parent) return p->grandchild;
        }
        return -1;
    }

    static pid_t getController(pid_t grandchild){
        GSList *list;
        for (list = tuboList; list && list->data; list=list->next){
            tuboPublic_t *p = (tuboPublic_t *)list->data;
            if (p->grandchild == grandchild) return p->PID;
        };
        return -1;
    }


    // We cannot get rid of grandchild fork because we need two processes 
    // at the terminating end of the pipes. This way we will not lose any
    // output on broken pipe conditions.
    //static unsigned Tubo_id(void){ 

    // This returns the last running instance counter id
    static gint getInstance(void){ 
        ERROR("Do you want Tubo_id() or would Tubo_get_id(pid) be what you want?\n");
        return instance;
    }


    //static unsigned Tubo_get_id(pid_t pid){ 
    //
    //This returns the instance id according to the process pid.
    static gint getInstance(pid_t PID){
        gint id = 0;
        pthread_mutex_lock(&list_mutex);
        GSList *list;
        for(list = tuboList; list && list->data; list=list->next){
            tuboPublic_t *p = (tuboPublic_t *)list->data;
            if (PID == p->PID || PID == p->grandchild){
                pthread_mutex_unlock(&list_mutex);
                return p->id;
            }
        }
        pthread_mutex_unlock(&list_mutex);
        return id;
    }

private:
    static void publicInsert(pid_t PID, pid_t grandchild, int instance_in){
        pthread_mutex_lock(&list_mutex);
        tuboPublic_t *item = (tuboPublic_t *)calloc(1, sizeof(tuboPublic_t));
        if (!item) {
            std::cerr<<"*** publicInsert(): calloc failed\n"; 
            exit(1);
        }       
        item->PID = PID;
        item->grandchild = grandchild;
        item->next = NULL;
        tuboList = g_slist_prepend(tuboList, item);      
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    static void publicRemove(pid_t PID, pid_t grandchild, int instance_in){
        pthread_mutex_lock(&list_mutex);
        GSList *list = tuboList;
        for (;list && list->data; list= list->next){
            tuboPublic_t *item = (tuboPublic_t *)list->data;
            if (item->PID == PID) {
                tuboList = g_slist_remove(tuboList, list->data);
                pthread_mutex_unlock(&list_mutex);
                return;
            }
        }
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    ////////   remote semaphore section. /////////
# define HEAD_SHM "/Tubo" 
    static gchar *
    shmName(gint parent, gint instance_in){
        return g_strdup_printf("%s-%u-%d", HEAD_SHM, parent, instance_in);
    }

    static gboolean
    setShmName(char *target, const char *tag, int parent, int instance_in){
        gchar *shm_name = shmName(parent, instance_in);
        if (!shm_name) return FALSE;
        if (strlen(shm_name) + strlen(tag) >= 64){
            fprintf(stderr, "*** Tubo: shm_name too long: %s\n", shm_name);
            g_free(shm_name);
            return FALSE;
        }
        // FIXME: use g_strconcat() and return gchar *
        //        then we need to g_free on destruction of data pointer
        strcpy(target, shm_name);
        strcat(target, tag);
        g_free(shm_name);
        return TRUE;
    }
 

    // 16 K, size of a memory page in  silicon graphics- cray boxes.
    // (just a bit of ancient history)
#define TUBO_BLK_SIZE 128*128

    static gboolean validAnsiSequence(gchar *line){
        const gchar *invalid_sequence_p[] =
        { "(", ")", "M", "E", "7", "8", "H", "[A", "[g", "[0g", "[3g", "#3", "#4", "#5", "#6", "5n", "6n", "[c", NULL };
        gchar *esc=line;

        while (esc != NULL && *esc != 0) {
            esc=strchr(esc,0x1b);
            if (esc==NULL || *esc==0) return TRUE;
            esc++;
            const gchar **p;
            for(p = invalid_sequence_p; p && *p; p++) {
                if (strncmp(esc,*p,strlen(*p))==0){
                    ERROR( "*** Tubo: sequence <ESC>%s is not in validAnsiSequence list\n", *p);
                    return FALSE;
                }
            }
        }
        return TRUE;
    }

    // grandchildPID is global static, but each fork has its own copy
    // FIXME: this is not elegant...
    static void signalIt (gint sig) {
        TRACE(" got signal %d (SIGUSR1=%d,SIGUSR2=%d)\n", sig, SIGUSR1, SIGUSR2);
        if(sig == SIGUSR1) kill (grandchildPID, SIGTERM);
        if(sig == SIGUSR2) kill (grandchildPID, SIGKILL);
        return;
    }

    static void signalConnections (void ) {
    //    signal(SIGHUP, signalIt);
    //    signal (SIGTERM, signalIt);
        signal (SIGUSR1, signalIt);
        signal (SIGUSR2, signalIt);
    }

    static void closePipes (forkStruct_t * forkO) {
        gint i;
        TRACE (" ClosePipes()\n");
        if(!forkO) {
            return ;
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
        return ;
    }

    static gboolean readFD (gint which, void * data) {
        static pthread_mutex_t user_f_mutex = PTHREAD_MUTEX_INITIALIZER;
        forkStruct_t *fork_p = (forkStruct_t *) data;
        gchar line[TUBO_BLK_SIZE];
        memset (line, 0, TUBO_BLK_SIZE);
        if(which < 1 || which > 2){
            ERROR( "*** Tubo:  readFD(): argument out of range\n");
            return FALSE;
        }

        for(gint i = 0; i < TUBO_BLK_SIZE - 2; i++) {
            // memset (line, 0, TUBO_BLK_SIZE); above will null terminate the string.
            // coverity[string_null_argument : FALSE]
            int result = read (fork_p->tubo[which][0], line + i, 1);
            if(result < 0) {
                ERROR( "*** Tubo:  readFD(%d->%d) %s\n", which, fork_p->tubo[which][0], strerror (errno));
                return FALSE;
            }
            if(result == 0) {
                return FALSE;
            }
            if(*(line + i) == '\n') {
                *(line + i + 1) = (char)0;
                break;
            }
        }

    // check for valid output (if so configured)
        if (fork_p->flags & TUBO_VALID_ANSI) {
            if (!validAnsiSequence(line)){
                ERROR( "*** Tubo: Received !validAnsiSequence\n");
		ERROR("Sending SIGTERM to child process (flags & TUBO_REAP_CHILD==TRUE)\n");
                // send child TERM signal: SIGUSR1 gets 
                // translated to SIGTERM for grandchild
                kill (fork_p->grandchild, SIGTERM); 	    
                // ignore data
                return TRUE;
            }
        }

        pthread_mutex_lock(&user_f_mutex);

        if(which == 2 && fork_p->stderr_f) {
            (*(fork_p->stderr_f)) (fork_p->user_function_data, (void *)line, fork_p->tubo[0][1]);
        } else if(which == 1 && fork_p->stdout_f) {
            (*(fork_p->stdout_f)) (fork_p->user_function_data, (void *)line, fork_p->tubo[0][1]);
        }
        pthread_mutex_unlock(&user_f_mutex);
        return TRUE;

    }

    static void stdio_f (gint which, void * data) {
        forkStruct_t *fork_p = (forkStruct_t *) data;

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
            sem_post(fork_p->stdout_sem);
            //{int v;_sem_getvalue(fork_p->stdout_sem, &v);fprintf(stderr,"sem %s after post, value=%d\n", fork_p->stdout_sem_name,v);fflush(stderr);} 

            // This is racy in Windows, since close implies unlink as well.
            // So in windows we defer close later on.
            sem_close(fork_p->stdout_sem);

        } else {
            TRACE("****: thread 0x%x posting semaphore %s\n", getpid(), fork_p->stderr_sem_name);
            //{int v;_sem_getvalue(fork_p->stdout_sem, &v);fprintf(stderr,"sem %s before post, value=%d\n", fork_p->stderr_sem_name,v);fflush(stderr);} 
            sem_post(fork_p->stderr_sem);
            //{int v;_sem_getvalue(fork_p->stdout_sem, &v);fprintf(stderr,"sem %s after post, value=%d\n", fork_p->stderr_sem_name,v);fflush(stderr);} 
            // This is racy in Windows, since close implies unlink as well.
            // So in windows we defer close later on.
            sem_close(fork_p->stderr_sem);
        }

        do {
            if (!(readFD (which, (void *)fork_p))) break;
        } while (1);
        //while(readFD (which, (void *)fork_p)) ;

        TRACE( " closing fd(%d)-> %d\n", which, fork_p->tubo[which][0]);
        close (fork_p->tubo[which][0]);
        // greenlight to fork_finished function:
        TRACE( "stdio_f posting local semaphore %d (%p)\n", which, (void *) (fork_p->local_semaphore + which));
            //{int v;_sem_getvalue(fork_p->local_semaphore + which, &v);fprintf(stderr,"sem %p before post, value=%d\n", (void *)(fork_p->local_semaphore + which),v);fflush(stderr);} 
        sem_post (fork_p->local_semaphore + which);
            //{int v;_sem_getvalue(fork_p->local_semaphore + which, &v);fprintf(stderr,"sem %p after post, value=%d\n", (void *)(fork_p->local_semaphore),v);fflush(stderr);} 
        TRACE( "Sem is now posted.\n");
    }

    static void *stdoutThread_f (void * data) {
        stdio_f (1, data);
        TRACE( "oooo  stdoutThread_f is now complete...\n");
        return NULL;
    }

    static void *stderrThread_f (void * data) {
        stdio_f (2, data);
        TRACE( "oooo  stderrThread_f is now complete...\n");
        return NULL;
    }

    static void *threadedWait_f (void * data) {
        forkStruct_t *fork_p = (forkStruct_t *) data;
        int status;
        //map_remote_semaphores(fork_p);
        TRACE ("....  thread=0x%x, wait for 0x%x\n", (unsigned)getpid (), (unsigned)(fork_p->PID));

        if(fork_p->flags & TUBO_REAP_CHILD) {
            waitpid (fork_p->PID, &status, 0);
        } 
        else {

            // leave child in waitable state...
            // no such thing in openBSD (P_PID will not be defined)
#ifdef P_PID
            siginfo_t infop;
            waitid (P_PID, fork_p->PID, &infop, WNOWAIT);
#endif
        }

        // TRACE (" threadedWait_f waiting for semaphore 0 (stdin)...\n");
        // sem_wait (fork_p->local_semaphore); // go ahead with stdin
        // sem_close(fork_p->local_semaphore);
        if (fork_p->stderr_f) {
            TRACE (" threadedWait_f waiting for local semaphore 2 (stderr)...\n");
            //{int v;_sem_getvalue((fork_p->local_semaphore + 2), &v);fprintf(stderr,"sem %p before wait, value=%d\n", (void *)(fork_p->local_semaphore + 2),v);fflush(stderr);} 
            sem_wait (fork_p->local_semaphore + 2);
            //{int v;_sem_getvalue((fork_p->local_semaphore + 2), &v);fprintf(stderr,"sem %p after wait, value=%d\n", (void *)(fork_p->local_semaphore + 2),v);fflush(stderr);} 
            sem_close(fork_p->local_semaphore + 2);
            sem_destroy(fork_p->local_semaphore + 2);
        }
        if (fork_p->stdout_f){
            TRACE (" threadedWait_f waiting for local semaphore 1 (stdout)...\n");
            //{int v;_sem_getvalue((fork_p->local_semaphore + 1), &v);fprintf(stderr,"sem %p before wait, value=%d\n", (void *)(fork_p->local_semaphore + 1),v);fflush(stderr);} 
            sem_wait (fork_p->local_semaphore + 1);    
            //{int v;_sem_getvalue((fork_p->local_semaphore + 1), &v);fprintf(stderr,"sem %p after wait, value=%d\n", (void *)(fork_p->local_semaphore + 1),v);fflush(stderr);} 
            sem_close(fork_p->local_semaphore + 1);
            sem_destroy(fork_p->local_semaphore + 2);
        }
        TRACE( " semaphores OK!\n");
        g_free (fork_p->local_semaphore);

        if(fork_p->tubo_done_f) {
            TRACE(" running tubo_done_f() now...\n");
            (*(fork_p->tubo_done_f)) (fork_p->user_function_data);
        }
        // finally, close stdin. 
        if(fork_p->tubo[0][1] > 0)
            close (fork_p->tubo[0][1]);


        TRACE( "oooo  threadedWait_f complete for 0x%x\n", (unsigned)(fork_p->PID));

        
        pthread_mutex_lock(&(fork_p->done_mutex));
        do {
            pthread_cond_wait(&(fork_p->done_signal),&(fork_p->done_mutex)); 
        } while (fork_p->done_value < 2);
        pthread_mutex_unlock(&(fork_p->done_mutex)); 

        g_free (fork_p);
        return NULL;
    }



    static void controller(forkStruct_t *newfork, int forked, int instance_in, pid_t parent){
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
            if (!setShmName(newfork->stdout_sem_name, "-stdout", parent, instance_in)) _exit(123);
            // open named sem
            newfork->stdout_sem = sem_open(newfork->stdout_sem_name, 0, 0700, 0);
            if (newfork->stdout_sem == SEM_FAILED) {
                ERROR( "*** Tubo: Cannot open named semaphore: %s (%s)\n", 
                        newfork->stdout_sem_name, strerror(errno));
            } else {
                // wait on remote sem
                TRACE("----: child wait for named semaphore %s...\n", newfork->stdout_sem_name);
                //{int v;_sem_getvalue(newfork->stdout_sem, &v);fprintf(stderr,"sem %s before wait, value=%d\n", newfork->stdout_sem_name,v);fflush(stderr);} 
                sem_wait(newfork->stdout_sem);
                //{int v;_sem_getvalue(newfork->stdout_sem, &v);fprintf(stderr,"sem %s after wait, value=%d\n", newfork->stdout_sem_name,v);fflush(stderr);} 
                TRACE("++++: child got semaphore %s\n", newfork->stdout_sem_name);   
                // close remote sem
                sem_close(newfork->stdout_sem);
                // remove remote sem (no longer in use)
                sem_unlink(newfork->stdout_sem_name);
            }
        }
        // wait on stderr_sem
        if(newfork->stderr_f != NULL) {
            if (!setShmName(newfork->stderr_sem_name, "-stderr", parent, instance_in)) _exit(123);
            // open named sem
            newfork->stderr_sem = sem_open(newfork->stderr_sem_name, 0, 0700, 0);
            if (newfork->stderr_sem == SEM_FAILED) {
                ERROR( "*** Tubo: Cannot open named semaphore: %s (%s)\n", 
                        newfork->stderr_sem_name, strerror(errno));
            } else {
                // wait on remote sem
                TRACE("----: child wait for named semaphore %s...\n", newfork->stderr_sem_name);
                //{int v;_sem_getvalue(newfork->stderr_sem, &v);fprintf(stderr,"sem %s before wait, value=%d\n", newfork->stderr_sem_name,v);fflush(stderr);} 
                sem_wait(newfork->stderr_sem);
                //{int v;_sem_getvalue(newfork->stderr_sem, &v);fprintf(stderr,"sem %s after wait, value=%d\n", newfork->stderr_sem_name,v);fflush(stderr);} 
                TRACE("++++: child got semaphore %s\n", newfork->stderr_sem_name);   
                // close remote sem
                sem_close(newfork->stderr_sem);
                // remove remote sem (no longer in use)
                sem_unlink(newfork->stderr_sem_name);
            }
         }

        if (!setShmName(newfork->setup_sem_name, "-setup", parent, instance_in)) _exit(123);
        newfork->setup_sem = sem_open(newfork->setup_sem_name, 0, 0700, 0);
        if (newfork->setup_sem == SEM_FAILED) {
            ERROR( "*** Tubo: Cannot open named semaphore: %s (%s)\n", 
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
                ERROR( "Tubo_thread incorrect usage: fork_function must _exit()\n");
                _exit (123);
            } else {
                TRACE( "grandchild here B... \n");
                TRACE( "* execvp %s\n", newfork->argv[0]);
                execvp(newfork->argv[0], newfork->argv);
                ERROR( "*** Tubo:  Cannot execvp %s (%s)\n", newfork->argv[0],strerror(errno));
                _exit(123);
            }
        } 
            
        // put grandchild pid into shm 
        if (!setShmName(newfork->shm_gchild_name, "-gchild", parent, instance_in))_exit(123);
        
        gint fd = 
            shm_open(newfork->shm_gchild_name, O_RDWR, 0700);
        if(fd < 0){
            ERROR( "*** Tubo: child shm open(%s): %s\n", newfork->shm_gchild_name, strerror (errno));
        } else {
                TRACE( "C>>>> grandchild id is 0x%x\n", grandchildPID);
                pid_t *gchild = 
                    (pid_t *)mmap(NULL, sizeof (pid_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                close(fd);
                if (gchild == MAP_FAILED){
                    ERROR( "*** Tubo: child mmap failed for %s: %s\n", 
                            newfork->shm_gchild_name, strerror (errno));
                } else{
                    *gchild = grandchildPID;
                    if(msync (gchild, sizeof(pid_t), MS_SYNC) < 0){
                        ERROR( "*** Tubo: Child msync(%s): %s\n", newfork->shm_gchild_name, strerror (errno));
                    }
                    munmap (gchild, sizeof(pid_t));
                }
        }    
        

        TRACE("****: child posting semaphore %s\n", newfork->setup_sem_name);
            //{int v;_sem_getvalue(newfork->stdout_sem, &v);fprintf(stderr,"sem %s before post, value=%d\n", newfork->setup_sem_name,v);fflush(stderr);} 
        if (newfork->setup_sem != SEM_FAILED){
            sem_post(newfork->setup_sem);
                //{int v;_sem_getvalue(newfork->stdout_sem, &v);fprintf(stderr,"sem %s after post, value=%d\n", newfork->setup_sem_name,v);fflush(stderr);} 
            // It is okay to close this here (in Windows) since the handle
            // is dupped in the parent.
            sem_close(newfork->setup_sem);
        }
        signalConnections ();  // pass signals to the grandchild (disabled in windoze).

        gint status;
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
        g_free(newfork);
        TRACE( "OOOO  controller process now complete\n");


        if (forked) _exit (123);

        return;

    }


    static void parentSetup(forkStruct_t *newfork)    {      

            // create a condition so that finished function will
            // not start until all threads are done.

            // Local semaphores (or conditions)
            TRACE("mallocing local semaphores...\n");
            newfork->local_semaphore = (sem_t *) malloc (3 * sizeof (sem_t));
            if (!newfork->local_semaphore){
                ERROR( "*** Tubo: malloc(%lu) %s\n", 
                        (long unsigned)(3 * sizeof (sem_t)), strerror(errno));
                return;
            }
            TRACE("initializing local semaphores...\n");
            //sem_init (newfork->local_semaphore, 0, 1); // go ahead with stdin
            if (newfork->stdout_f) sem_init (newfork->local_semaphore + 1, 0, 0);
            if (newfork->stderr_f) sem_init (newfork->local_semaphore + 2, 0, 0);        
            TRACE("initialized local semaphores...\n");

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
                pthread_create(&thread, NULL, stdoutThread_f, (void *)newfork);
                pthread_detach(thread);
            }
            /* stderr for read: */
            if(newfork->stderr_f == NULL) {
                // greenlight to fork_finished function:
                if (newfork->tubo[2][0]>0){
                        ERROR( "*** Tubo: file descriptor 2-r should not be open\n");
                }
                newfork->done_value++;
            } else {
                TRACE (" parent creating stderr thread\n");
                pthread_t thread;
                pthread_create(&thread, NULL, stderrThread_f, (void *)newfork);
                pthread_detach(thread);
            }

           
            /* fire off a threaded wait for the child */
            pthread_t thread;
            pthread_create(&thread, NULL, threadedWait_f, (void *)newfork);
            pthread_detach(thread);

            /* threads are now in place and ready to read from pipes,
             * child process will get green light to exec
             * as soon as stdin/stdout threads are ready... */

            TRACE( "----: parent process waiting for semaphore %s\n", newfork->setup_sem_name);
            //{int v;_sem_getvalue(newfork->setup_sem, &v);fprintf(stderr,"sem %s before wait, value=%d\n", newfork->setup_sem_name,v);fflush(stderr);} 
            sem_wait(newfork->setup_sem);
            //{int v;_sem_getvalue(newfork->setup_sem, &v);fprintf(stderr,"sem %s after wait, value=%d\n", newfork->setup_sem_name,v);fflush(stderr);} 
            TRACE( "++++: parent process got semaphore %s\n", newfork->setup_sem_name);
            // Note: windows will unlink semaphore automatically when all referenced
            //       handles are closed. This behavior creates a potential race
            //       condition which must be taken into account.
            //       Here it is OK to remove semaphore.
            
            sem_close(newfork->setup_sem);
            sem_unlink(newfork->setup_sem_name);

            // If stdout and stderr are done with, we give the child
            // a green light to exit when ready. Otherwise, stdio_f
            // will send the green light in due time.

            
            // Get the grandchild pid from shared memory:
            // 
            newfork->grandchild = *(newfork->gchild_p);
            munmap (newfork->gchild_p, sizeof(pid_t));
            shm_unlink(newfork->shm_gchild_name);
            TRACE( "P>>>> grandchild id is 0x%x\n", newfork->grandchild);

            publicInsert(newfork->PID, newfork->grandchild, newfork->tubo_id);
            
            TRACE( " The parent process is returning, grandchild 0x%x\n", newfork->grandchild);
            

            TRACE("  parent %d<->%d   %d<->%d   %d<->%d\n",
                   newfork->tubo[0][0],
                   newfork->tubo[0][1], newfork->tubo[1][0], newfork->tubo[1][1], newfork->tubo[2][0], newfork->tubo[2][1]);
            return;    
    }



    static gboolean restoreFD(gint *save_p, gint original){
        if (*save_p != -1) {
          if(dup2(*save_p, original) < 0){
              ERROR("*** Tubo: cannot restore %d\n", original);
              close(*save_p);        
              return FALSE;
          }
          close(*save_p);        
        }
        return TRUE;
    }

    static gboolean setupPipes1(forkStruct_t *newfork, gint *save_stdin_p, gint *save_stdout_p, gint *save_stderr_p){

        // Always save stdin, because if it is not piped, then it is set to /dev/null
        gint in_fd = fileno(stdin);
        if (in_fd >= 0) *save_stdin_p = dup(in_fd);
        else *save_stdin_p = -1;

        if (newfork->stdin_fd != -1) {
            // This sets stdin straight from the pipe.
            if (dup2(newfork->tubo[0][0], fileno(stdin)) < 0){
                // unable to create pipe
                ERROR( "*** Tubo: unable to create stdin pipe with dup2\n");
                restoreFD(save_stdin_p, fileno(stdin));
                return FALSE;
            }
            close(newfork->tubo[0][0]);
            newfork->tubo[0][0] = -1;
            // setup semaphore will control stdin...
        } else {
            // Nothing interesting in stdin, so don't find anything...
            gint fd = in_fd;
            if (fd >= 0){
                newfork->nullfd = fopen ("/dev/null", "rb");
                if (dup2 (fileno (newfork->nullfd), fd) < 0){
                    // unable to create pipe
                    ERROR( "*** Tubo: unable to create stdin pipe with dup2\n");
                    restoreFD(save_stdin_p, fileno(stdin));

                    return FALSE;
                }
            }
        }


        *save_stdout_p = -1;
        if (newfork->stdout_f) {
            // This sets stdout straight for the pipe.
            *save_stdout_p = dup(fileno(stdout));
            if (dup2(newfork->tubo[1][1], fileno(stdout)) < 0){
                // unable to create pipe
                restoreFD(save_stdin_p, fileno(stdin));
                restoreFD(save_stdout_p, fileno(stdout));
                ERROR( "*** Tubo: unable to create stdout pipe with dup2\n");
                
                return FALSE;
            }
            close(newfork->tubo[1][1]);
            newfork->tubo[1][1] = -1;
            if (!setShmName(newfork->stdout_sem_name, "-stdout", getpid(), instance))return 0;
            newfork->stdout_sem = sem_open(newfork->stdout_sem_name, O_CREAT|O_EXCL, 0700, 0);
            if (newfork->stdout_sem == SEM_FAILED) {
                restoreFD(save_stdin_p, fileno(stdin));
                restoreFD(save_stdout_p, fileno(stdout));
                ERROR( "*** Tubo: Cannot create named semaphore: %s (%s)\n", 
                        newfork->stdout_sem_name, strerror(errno));
                return FALSE;
            }
            
        } 

        *save_stderr_p = -1;
        if (newfork->stderr_f) {
            // This sets stderr straight for the pipe.
            *save_stderr_p = dup(fileno(stderr));
            if (dup2(newfork->tubo[2][1], fileno(stderr)) < 0){
                // unable to create pipe
                restoreFD(save_stdin_p, fileno(stdin));
                restoreFD(save_stderr_p, fileno(stderr));
                restoreFD(save_stdout_p, fileno(stdout));
                ERROR("unable to create stderr pipe with dup2\n");
                return FALSE;
            }
            close(newfork->tubo[2][1]);
            newfork->tubo[2][1] = -1;
            if (!setShmName(newfork->stderr_sem_name, "-stderr", getpid(), instance))return 0;
            newfork->stderr_sem = sem_open(newfork->stderr_sem_name, O_CREAT|O_EXCL, 0700, 0);
            if (newfork->stderr_sem == SEM_FAILED) {
                restoreFD(save_stdin_p, fileno(stdin));
                restoreFD(save_stdout_p, fileno(stdout));
                restoreFD(save_stderr_p, fileno(stderr));
                ERROR( "*** Tubo: Cannot create named semaphore: %s (%s)\n", 
                        newfork->stderr_sem_name, strerror(errno));
                return FALSE;
            }
        }

        if (!setShmName(newfork->setup_sem_name, "-setup", getpid(), instance))return 0;
        newfork->setup_sem = sem_open(newfork->setup_sem_name, O_CREAT|O_EXCL, 0700, 0);
        if (newfork->setup_sem == SEM_FAILED) {
                restoreFD(save_stdin_p, fileno(stdin));
                restoreFD(save_stdout_p, fileno(stdout));
                restoreFD(save_stderr_p, fileno(stderr));
            ERROR( "*** Tubo: Cannot create named semaphore: %s (%s)\n", 
                    newfork->setup_sem_name, strerror(errno));
                return FALSE;
        }
        if (!setShmName(newfork->shm_gchild_name, "-gchild", getpid(), instance))return 0;
        gint fd = shm_open(newfork->shm_gchild_name, O_RDWR | O_CREAT | O_EXCL, 0700);
        if(fd < 0){
                restoreFD(save_stdin_p, fileno(stdin));
                restoreFD(save_stdout_p, fileno(stdout));
                restoreFD(save_stderr_p, fileno(stderr));
            ERROR( "*** Tubo: parent shm open(%s): %s\n", newfork->shm_gchild_name, strerror (errno));
                return FALSE;
        } else {
            if(ftruncate (fd, sizeof (pid_t)) < 0) {
                restoreFD(save_stdin_p, fileno(stdin));
                restoreFD(save_stdout_p, fileno(stdout));
                restoreFD(save_stderr_p, fileno(stderr));
                ERROR( "*** Tubo: ftruncate(%s): %s\n", newfork->shm_gchild_name, strerror (errno));
                close(fd);
                return FALSE;
            }
            newfork->gchild_p = 
                    (pid_t *)mmap(NULL, sizeof (pid_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            close(fd);
            if ( newfork->gchild_p == MAP_FAILED){
                restoreFD(save_stdin_p, fileno(stdin));
                restoreFD(save_stdout_p, fileno(stdout));
                restoreFD(save_stderr_p, fileno(stderr));
                 ERROR( "*** Tubo: mmap failed for %s: %s\n", 
                    newfork->shm_gchild_name, strerror (errno));
                return FALSE;
            } 

        }

        return TRUE;
    }


    static gboolean setupPipes2(forkStruct_t *newfork, gint save_stdin, gint save_stdout, gint save_stderr){
        // This function is executed by the parent after the fork or spawn
        // Now we must reset stdout and stderr to saved values.
        if (newfork->nullfd) fclose(newfork->nullfd);
        restoreFD(&save_stdin, fileno(stdin));
        restoreFD(&save_stdout, fileno(stdout));
        restoreFD(&save_stderr, fileno(stderr));
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

        return TRUE;
    }

#define PIPE(x) pipe(x)

    static forkStruct_t * forkStructNew(
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
        forkStruct_t *newfork = (forkStruct_t *)calloc(1, sizeof(forkStruct_t));
        if (!newfork) {ERROR("*** Tubo: calloc: %s\n", strerror(errno)); return NULL;}

        instance++;
        newfork->flags = flags;

        for(gint i = 0; i < 3; i++) newfork->tubo[i][0] = newfork->tubo[i][1] = -1;

        if (stdin_fd_p){
            if(PIPE(newfork->tubo[0]) == -1 || newfork->tubo[0][0] <= 2  || newfork->tubo[0][1] <= 2) {
                ERROR("Incorrect pipes (0): %d <-> %d \n", newfork->tubo[0][0], newfork->tubo[0][1]); 
                closePipes (newfork);
                newfork->stdin_fd = -1;
                g_free(newfork);
                return NULL;
            }
            *stdin_fd_p = newfork->tubo[0][1];
            newfork->stdin_fd = *stdin_fd_p;
        } else {
            newfork->stdin_fd = -1;
        }

        if (stdout_f){
            if(PIPE(newfork->tubo[1]) == -1 || newfork->tubo[1][0] <= 2  || newfork->tubo[1][1] <= 2) {
                ERROR("Incorrect pipes (1): %d <-> %d \n", newfork->tubo[1][0], newfork->tubo[1][1]); 
                closePipes (newfork);
                g_free(newfork);
                return NULL;
            }

        }

        if (stderr_f){
            if(PIPE(newfork->tubo[2]) == -1 || newfork->tubo[2][0] <= 2  || newfork->tubo[2][1] <= 2) {
                ERROR("Incorrect pipes (2): %d <-> %d \n", newfork->tubo[2][0], newfork->tubo[2][1]); 
                closePipes (newfork);
                g_free(newfork);
                return NULL;
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

};
}



#endif
