/*   tubo.h */

/*   
 *  Copyright (C) 2000-2012  Edscott Wilson Garcia under GNU GPLv3
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc.
*/
/* public stuff */

/* Tubo() returns void pointer to tubo object 
*  usage: see example below */
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#ifdef __cplusplus
extern "C" {
#endif

#ifndef TUBO_DISABLE_DEPRECATED

/**
 * Tubo_threads:
 * @fork_function: pointer to function to execute after forking. This will 
 * execute in the child process, and upon termination will be reaped by
 * the controller process. 
 * @fork_function_data: pointer to data to be sent to @fork_function, or NULL.
 * @stdin_fd: pointer for file descriptor for stdin, or NULL
 * @stdout_f: pointer to thread safe function to process stdout, or NULL. This
 * is executed by an independent thread. Please note that gtk or qt instructions
 * should be done via main thread, as with g_main_context_invoke(). Otherwise,
 * calling program is responsible for any necessary mutex locks.
 * @stderr_f: pointer to thread safe function to process stderr, or NULL .This
 * is executed by an independent thread. Please note that gtk or qt instructions
 * should be done via main thread, as with g_main_context_invoke(). Otherwise,
 * calling program is responsible for any necessary mutex locks.
 * @tubo_done_f: pointer to function to execute when remote process has terminated.
 * Execution of this function does not mean all pipe data has been processed
 * yet (data may still be flowing in the pipes). It just means that the spawned
 * process has exited.
 * @user_function_data: pointer to data to be sent to @stdout_f, @stderr_f 
 * and @tubo_done_f (yeah, same data block for all three).
 * @flags  TUBO_EXIT_TEXT: print a process termination text at the end of stdout.
 * @flags  TUBO_REAP_CHILD: flag to reap child. Set to TRUE to have the controller
 * process reaped.
 * If this is set to FALSE, then the user is responsible to reap the controller
 * process (i.e., with waitpid()) to avoid leaving a zombie. The controller
 * process is responsible for reaping the child program.
 * @flags TUBO_REAP_CHILD: flag to filter out non printer safe output.
 * If set to TRUE, output to stdout/stderr will be checked
 * for escape sequences that are not valid on a printer terminal and if any such
 * sequence is found, the child will be terminated without much more ado.
 * 
 * Returns: pid of controller process. This pid is not the pid of the program
 * being executed. It is a process that ensures pipes will remain open while
 * pipe output is still to be read. To send a TERM signal to the actual
 * process which is executing the remote command, send a USR1 signal to the
 * controller process. To send a KILL to the actual process which is executing
 * the remote command, send a USR1 signal to the controller process. Finer
 * signalling requires the use of Tubo_child() function. 
 *
 * 		DEPRECATED: Use Tubo_fork() or Tubo_exec() instead
 **/

pid_t Tubo_threads (
    void (*fork_function) (void *),
    void *fork_function_data,
    int *stdin_fd,
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
    );
#endif

/**
 * Tubo_id:
 *
 * Returns: Incremental serial number of last spawned process. This function
 * allows the calling program to keep track of how many processes have been
 * spawned since library was loaded by the calling program.
 *
 **/

unsigned 
Tubo_id(void);


/**
 * Tubo_get_id:
 * @pid: pid of the program being executed or of the controller process.
 *
 * Returns: Incremental serial number of the program being executed 
 * where pid or controller pid matches @pid. This function allows the
 * calling program obtain the internal serial number for any process 
 * running within the current Tubo session.
 *
 **/

unsigned 
Tubo_get_id(pid_t pid);


/**
 * Tubo_child:
 * @controller: pid of the controller process. This pid is not the pid of the 
 * program being executed. It is the pid of the controller process.
 * 
 * Returns: pid of program being executed. This pid may be used to signal
 * directly to the process, instead of the limited SIGUSR1 and SIGUSR2
 * described in Tubo_threads(). Signals are currently not available in Windows.
 * 
 **/
 
int
Tubo_child(pid_t controller);

/**
 * Tubo_controller:
 * @child: pid of the program being executed. 
 *
 * Returns: pid of the process controller. This pid may be used to signal
 * to the program with SIGUSR1 and SIGUSR2.
 * Signals are currently not available in Windows.
 *  
 **/
 
int
Tubo_controller(pid_t child);

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
 * Use this flag to have Tubo_exec() and Tubo_fork() return the controller
 * pid instead of the pid of the program being executed. This is the default
 * in Tubo_threads().
 *
 **/
#define TUBO_CONTROLLER_PID  0x08

/**
 * Tubo_fork:
 * @fork_function: pointer to function to execute after forking. This will 
 * execute in the child process, and upon termination will be reaped by
 * the controller process. 
 * @fork_function_data: pointer to data to be sent to @fork_function, or NULL.
 * @stdin_fd: pointer for file descriptor for stdin, or NULL
 * @stdout_f: pointer to thread safe function to process stdout, or NULL. This
 * is executed by an independent thread. Please note that gtk or qt instructions
 * should be done via main thread, as with g_main_context_invoke(). Otherwise,
 * calling program is responsible for any necessary mutex locks.
 * @stderr_f: pointer to thread safe function to process stderr, or NULL .This
 * is executed by an independent thread. Please note that gtk or qt instructions
 * should be done via main thread, as with g_main_context_invoke(). Otherwise,
 * calling program is responsible for any necessary mutex locks.
 * @tubo_done_f: pointer to function to execute when remote process has terminated.
 * Execution of this function does not mean all pipe data has been processed
 * yet (data may still be flowing in the pipes). It just means that the spawned
 * process has exited.
 * @user_function_data: pointer to data to be sent to @stdout_f, @stderr_f 
 * and @tubo_done_f (yeah, same data block for all three).
 * @flags  TUBO_EXIT_TEXT: print a process termination text at the end of stdout.
 * @flags  TUBO_REAP_CHILD: flag to reap child. Set to TRUE to have the controller
 * process reaped.
 * If this is set to FALSE, then the user is responsible to reap the controller
 * process (i.e., with waitpid()) to avoid leaving a zombie. The controller
 * process is responsible for reaping the child program.
 * @flags:  TUBO_EXIT_TEXT, print a process termination text at the end of 
 * stdout. TUBO_REAP_CHILD, flag to reap child. Set to TRUE to have the 
 * controller process reaped. If this is not set to, then the user is 
 * responsible to reap the controller process (i.e., with waitpid()) 
 * to avoid leaving a zombie. The controller process is responsible for
 * reaping the child program. TUBO_VALID_ANSI, flag to filter out non 
 * printer safe output. If set, output to stdout/stderr will be checked
 * for escape sequences that are not valid on a printer terminal and if any such
 * sequence is found, the child will be terminated without much more ado.
 * 
 * Returns: pid of the program (not the controller). To obtain the controller
 * pid use Tubo_controller() with the pid of the program as first argument.
 *
 * This function is not available in systems without fork() (i.e. windoze).
 *
 **/

pid_t Tubo_fork (
    void (*fork_function) (void *),
    void *fork_function_data,
    int *stdin_fd,
    void (*stdout_f) (void *stdout_data,
                      void *stream,
                      int childFD),
    void (*stderr_f) (void *stderr_data,
                      void *stream,
                      int childFD),
    void (*tubo_done_f) (void *),
    void *user_function_data,
    int flags
    );

/**
 * Tubo_exec:
 * @argv: NULL terminated string vector with the arguments of the program to 
 * be executed. This function uses spawnvp() on windows.
 * @stdin_fd: pointer for file descriptor for stdin, or NULL
 * @stdout_f: pointer to thread safe function to process stdout, or NULL. This
 * is executed by an independent thread. Please note that gtk or qt instructions
 * should be done via main thread, as with g_main_context_invoke(). Otherwise,
 * calling program is responsible for any necessary mutex locks.
 * @stderr_f: pointer to thread safe function to process stderr, or NULL .This
 * is executed by an independent thread. Please note that gtk or qt instructions
 * should be done via main thread, as with g_main_context_invoke(). Otherwise,
 * calling program is responsible for any necessary mutex locks.
 * @tubo_done_f: pointer to function to execute when remote process has terminated.
 * Execution of this function does not mean all pipe data has been processed
 * yet (data may still be flowing in the pipes). It just means that the spawned
 * process has exited.
 * @user_function_data: pointer to data to be sent to @stdout_f, @stderr_f 
 * and @tubo_done_f (yeah, same data block for all three).
 * @flags:  TUBO_EXIT_TEXT, print a process termination text at the end of 
 * stdout. TUBO_REAP_CHILD, flag to reap child. Set to TRUE to have the 
 * controller process reaped. If this is not set to, then the user is 
 * responsible to reap the controller process (i.e., with waitpid()) 
 * to avoid leaving a zombie. The controller process is responsible for
 * reaping the child program. TUBO_VALID_ANSI, flag to filter out non 
 * printer safe output. If set, output to stdout/stderr will be checked
 * for escape sequences that are not valid on a printer terminal and if any such
 * sequence is found, the child will be terminated without much more ado.
 * 
 * Returns: pid of the program (not the controller). To obtain the controller
 * pid use Tubo_controller() with the pid of the program as first argument.
 *
 * This function is available in Linux, BSD and Windows.
 *
 **/


pid_t Tubo_exec (
    char **argv,
    int *stdin_fd,
    void (*stdout_f) (void *stdout_data,
                      void *stream,
                      int childFD),
    void (*stderr_f) (void *stderr_data,
                      void *stream,
                      int childFD),
    void (*tubo_done_f) (void *),
    void *user_function_data,
    int flags
    );


#ifdef __cplusplus
}
#endif


/***********************************************************/
