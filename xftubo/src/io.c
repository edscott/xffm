#include <stdio.h>
#include <string.h>
#include <tubo.h>

int count=1;

static void 
stdout_f (void *user_data, void *stream, int childFD){
    char *line;
    line = (char *)stream;
    fprintf (stderr,"stdout_f childFD=%d (count=%d): %s \n", childFD, count++, line);
    if (childFD > 0){
        fprintf(stderr, "autoresponse...\n");
        char *response="auto response\n";
        write(childFD, response, strlen(response));
    }
    return;
}
    
static void
fork_f(void *fork_data){
    fprintf(stdout, "forking...\n");
    int i;
    for (i=0; i<10; i++) {
        fprintf(stdout, "message %d\n", i);
        fflush(stdout);
        char buffer[4000];
        fgets(buffer, 400, stdin);
        fprintf(stderr, "scanned: %s\n", buffer);

    }
    sleep(5);
    _exit(123);
    //char *arg[]={"./io.pl", NULL};
    //execvp(arg[0], arg);
    //fprintf(stderr, "cannot exec...\n");
}

int main(int argc, char **argv){
    int stdin_fd;
    char *a[]={"./io.pl", NULL};


    // here we could close p[0], not needed here.


/*    pid_t pid = Tubo_exec(
            a,
            &stdin_fd, 
            stdout_f, NULL,
            NULL,
            NULL,
            TUBO_CONTROLLER_PID
            );*/
    pid_t pid = Tubo_fork(
            fork_f, NULL,
            &stdin_fd, 
            stdout_f, NULL,
            NULL,
            NULL,
            TUBO_CONTROLLER_PID
            );
    fprintf(stderr, "waiting for %d, stdin_fd-write=%d\n", pid, stdin_fd);
    int status;
    waitpid(pid, &status, 0);

    return 1;
}

