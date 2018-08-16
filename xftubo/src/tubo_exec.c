
/*   tubo_exec.c  */

/*  A program independent fork (or spawn) module.
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


#define USE_LOCAL_SEMAPHORES
#include "tubo_exec.i"

// We cannot get rid of grandchild fork because we need two processes 
// at the terminating end of the pipes. This way we will not lose any
// output on broken pipe conditions.

unsigned 
Tubo_id(void){ 
    DBG("Do you want Tubo_id() or would Tubo_get_id(pid) be what you want?\n");
    return instance;
}


unsigned 
Tubo_get_id(pid_t pid){ 
    return tubo_get_instance(pid);
}


// Tubo_fork() is only valid in Linux/BSD, as it uses fork()
pid_t Tubo_fork (
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
    fork_struct *newfork = fork_struct_new(NULL, fork_function, fork_function_data,
                            stdin_fd_p, stdout_f, stderr_f, tubo_done_f,
                            user_function_data, flags);
    if (!newfork) return 0;

    int save_stdin = -1;
    int save_stdout = -1;
    int save_stderr = -1;

    if (!setup_pipes_1(newfork, &save_stdin, &save_stdout, &save_stderr)) {
        if (save_stdin != -1) close(save_stdin);
        if (save_stdout != -1) close(save_stdout);
        if (save_stderr != -1) close(save_stderr);
        free(newfork);
        return 0;
    }

        
    pid_t parent=getpid();
        
    // The main fork
    pid_t PID = fork ();
    newfork->PID = PID;
    if(PID == 0) { /* the child */
        controller(newfork, 1, instance, parent);
        _exit(123);
    }

    // Now we must reset stdout and stderr to saved values.
    if (!setup_pipes_2(newfork, save_stdin, save_stdout, save_stderr)) return 0;

    parent_setup(newfork);
    // Child threads are still using newfork

    pid_t grandchild = newfork->grandchild;
    if (flags & TUBO_CONTROLLER_PID) return PID; 
    return grandchild;
}

pid_t
Tubo_threads (void (*fork_function) (void *),
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
	      int reap_child,
	      int check_valid_ansi_sequence
) {
    DBG("Tubo_threads() is deprecated. Please use Tubo_exec() or Tubo_fork()\n"); 
    int flags = TUBO_EXIT_TEXT|TUBO_CONTROLLER_PID;
    if (reap_child) flags |= TUBO_REAP_CHILD;
    if (check_valid_ansi_sequence) flags |= TUBO_VALID_ANSI;
    return Tubo_fork(fork_function, fork_function_data, stdin_fd_p,
            stdout_f, stderr_f, tubo_done_f, user_function_data,
            flags);
  
}
int
Tubo_child(pid_t parent){
    tubo_public_t *p = tubo_public_head;
    if (!p) return -1;
    do {
        if (p->PID == parent) return p->grandchild;
        p = p->next;
    } while (p);
    return -1;
}

int
Tubo_controller(pid_t grandchild){
    tubo_public_t *p = tubo_public_head;
    if (!p) return -1;
    do {
        if (p->grandchild == grandchild) return p->PID;
        p = p->next;
    } while (p);
    return -1;
}

pid_t Tubo_exec (
    char **argv,
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

# ifdef LINUX_TEST  
    const char *tuboexec = "./tuboexec";
# else 
    const char *tuboexec = "tuboexec";  
# endif

    fork_struct *newfork = fork_struct_new(argv, NULL, NULL,
                            stdin_fd_p, stdout_f, stderr_f, tubo_done_f, 
                            user_function_data, flags);
    if (!newfork) return 0;

            
#ifdef LINUX_TEST
    char **a = argv;
            TRACE( "---- ");
            for (; a && *a; a++){
                TRACE( "\"%s\" ", *a);
            }
            TRACE( "\n");
#endif

    int save_stdin = -1;
    int save_stdout = -1;
    int save_stderr = -1;
    if (!setup_pipes_1(newfork, &save_stdin, &save_stdout, &save_stderr)) {
        if (save_stdin != -1) close(save_stdin);
        if (save_stdout != -1) close(save_stdout);
        if (save_stderr != -1) close(save_stderr);
        free(newfork);
        return 0;
    }

    int argc = 0;
    char instance_s[64];
    char flag_s[64];
    char ppid_s[64];

    int flag = 0;
    if (stdout_f) flag |= TUBO_STDOUT;
    if (stderr_f) flag |= TUBO_STDERR;
    sprintf(flag_s, "%d", flag);

    sprintf(instance_s, "%d", instance);
    sprintf(ppid_s, "%d", getpid());

    for (argc=0; argv[argc]; argc++);
    char *arg[argc+5];

    arg[0] = (char *)tuboexec;
    
    arg[1] = instance_s;
    arg[2] = flag_s;
    arg[3] = ppid_s;
    
    memcpy(arg+4, argv, argc*sizeof(char *));
    arg[argc+4] = NULL;

#ifdef LINUX_TEST  
    char **a;
            TRACE( "%d++++ ", argc);
            for (a = arg; a && *a; a++){
                TRACE( "\"%s\" ", *a);
            }
            TRACE( "\n");
#endif


    // The main fork/spawn
    TRACE( "All set up for the fork/execvp\n");
    newfork->PID = fork ();
    if(newfork->PID == 0) { /* the child */
        TRACE("0x%x execvp(%s ...)\n", getpid(), arg[0]);
        execvp(arg[0], arg);

        DBG( "*** Tubo_exec(): unable to execvp %s (%s).\n", arg[0], strerror(errno));
        DBG( "Now trying to execute %s in current directory.\n", arg[0]);
        arg[0] = "./tuboexec";
        execvp(arg[0], arg);

        DBG( "*** unable to execvp %s (%s).\n", arg[0], strerror(errno));
        DBG( "*** aborting fork()....\n");
        _exit(123);
    }

    // Now we must reset stdout and stderr to saved values.
    if (!setup_pipes_2(newfork, save_stdin, save_stdout, save_stderr)) return 0;

    parent_setup(newfork);
    TRACE( "all set up...\n");

    pid_t PID = newfork->PID;
    pid_t grandchild = newfork->grandchild;

    // newfork is still used by child threads.
    if (flags & TUBO_CONTROLLER_PID) return PID; 
    return grandchild;    
}

int 
tubo_main(int argc, char **argv){
// create fork structure
// do the pipe thing.
#ifdef DEBUG_TRACE
    char **p=argv;
    int count=0;
    for (; p && *p; p++){
        TRACE( "tubo_main: arg='%s' %s\n", *p, (count>3)?"*":"");
    }
#endif

    if (argc < 5) {
        DBG( "%s: insufficient parameters (%d < 5)\n", argv[0], argc);
        exit(1);
    } else {
        TRACE( "tubo_main: argc=%d\n", argc);
    }


    TRACE( "exec %s\n", argv[0]);
    
    int instance_in = atoi(argv[1]);
    int flags = atoi(argv[2]);
    int parent = atoi(argv[3]);


    fork_struct *newfork = (fork_struct *)malloc(sizeof(fork_struct));
    if (!newfork) {DBG("malloc: %s\n", strerror(errno)); return 0;}  
    memset (newfork, 0, sizeof (fork_struct));

    newfork->fork_function = NULL;
    if (flags & TUBO_STDOUT) {
        newfork->stdout_f = (void *)((long)1);
    }
    if (flags & TUBO_STDERR) {
        newfork->stderr_f = (void *)((long)1);
    }

    newfork->stdin_fd = -1;
    
    newfork->argv = argv+4;
    TRACE( ".....arvgv+4 is \"%s\"\n", argv[4]);

    controller(newfork, 0, instance_in, parent);

    TRACE("tubo_main now exiting!\n"); 
    //done!

    return 1;
}


