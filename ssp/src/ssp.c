#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_FD 1024  // System-specific maximum file descriptor count

// initialize struct to hold PID, process name, ssp_id, and status for each ssp object
typedef struct ssp {
    pid_t pid;
    char *proc_name;
    int status;
} ssp;

// initialize array to hold ssp_ids
ssp *ssp_ids;
int n_id;   // number of ssp processes
int cap_id; // capacity of ssp_ids array

// initialize array to hold subreaper adopted processes
ssp *sr_ids;
int sr_n_id;   // number of sr processes
int sr_cap_id; // capacity of sr_ids array


void ssp_init() {
    // initialize ssp_ids array
    ssp_ids = (ssp *)malloc(sizeof(ssp));
    n_id = 0;
    cap_id = 1;

    // initialize ssp_ids array
    sr_ids = (ssp *)malloc(sizeof(ssp));
    sr_n_id = 0;
    sr_cap_id = 1;

    // make it a subreaper
    if (prctl(PR_SET_CHILD_SUBREAPER, 1) == -1) {
        perror("prctl failed");
        exit(1);
    }
}

int ssp_create(char *const *argv, int fd0, int fd1, int fd2) {
    // create a new process through fork
    pid_t pid = fork();

    if (pid < 0){ // error with fork
        perror("Fork failed");
        exit(1);
    }

    // Child Process
    if(pid == 0) {
        // set file descriptors 0, 1, 2 to fd0, fd1, fd2, checking for errors
        if (dup2(fd0, 0) < 0 || dup2(fd1, 1) < 0 || dup2(fd2, 2) < 0) {
            perror("dup2 failed");
            exit(1);
        }        
        
        // TODO: Close all other file descriptors except 0, 1, and 2
        // this current one is incorrect, see lab handout
        for (int fd = 3; fd < MAX_FD; ++fd) {
            close(fd);
        }

        // call execvp(argv[0], argv) to run the process
        execvp(argv[0], argv);

        // If execvp returns, there was an error
        perror("execvp failed");
        exit(errno);
    }

    // Parent Process, tracking child process info

    // check if ssp_ids is big enough
    if (n_id >= cap_id){
        cap_id *= 2;
        ssp_ids = (ssp *)realloc(ssp_ids, cap_id * sizeof(ssp));
    }

    // set proc_name
    ssp_ids[n_id].proc_name = malloc(sizeof(argv[0]));
    strcpy(ssp_ids[n_id].proc_name, argv[0]);

    // set pid
    ssp_ids[n_id].pid = pid;

    // set status
    ssp_ids[n_id].status = -1;
    
    // increment n_id
    n_id ++;

    // add child process to the adopted list

    return n_id - 1;
}

int ssp_get_status(int ssp_id) {
    // check the current status of the pid if it is running ( == -1)
    int status;
    if(ssp_ids[ssp_id].status == -1) {
        // get the status
        waitpid(ssp_ids[ssp_id].pid, &status, WNOHANG);
    }

    // check if status needs to be updated
    // check if child terminated normally (if WIFEXITED(status) is nonzero)
    if (WIFEXITED(status)) {
        ssp_ids[ssp_id].status = WEXITSTATUS(status);
    }

    return ssp_ids[ssp_id].status;
}

void ssp_send_signal(int ssp_id, int signum) {
    // send signal to process given by the ssp_id using kill
    // if the return value of kill is nonzero, there is an error
    if (kill(ssp_ids[ssp_id].pid, signum) != 0) {
        perror("Failed to send signal");
        exit(errno);
    }
}

void ssp_wait() {
    // go through all ssp_ids and wait for exit, check exit status to make sure btw 0 and 255

    // initialize vars, pid (to hold current pid) and status (to hold waitpid status)
    pid_t pid;
    int status;

    // go through ssp_ids array
    for (int i = 0; i < n_id; i ++){
        // if status == -1, it is running styll
        if (ssp_ids[i].status == -1){
            // get pid
            pid = ssp_ids[i].pid;

            // waits for process given by pid to finish and puts the termination 
            waitpid(pid, &status, 0);

            // check if child terminated normally (if WIFEXITED(status) is nonzero)
            if (WIFEXITED(status)) {
                ssp_ids[i].status = WEXITSTATUS(status);
            } 
            // check if terminated by signal (if WIFSIGNALED(status) is nonzero)
            else if (WIFSIGNALED(status)) {
                ssp_ids[i].status = WTERMSIG(status) + 128;
            }
        }
    }
    
}

void ssp_print() {
    // set up just, which is the length of the CMD  
    int just = 3;

    // calculate just
    for (int i = 0; i < n_id; i ++){
        if (strlen(ssp_ids[i].proc_name) > just){
            just = strlen(ssp_ids[i].proc_name);
        }
    }

    printf("%7s %-*s %s\n", "PID", just, "CMD", "STATUS");

    for (int i = 0; i < n_id; i ++){
        printf("%7d %-*s %d\n", ssp_ids[i].pid, just, ssp_ids[i].proc_name, ssp_ids[i].status);
    }
}
