#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
You should output the PID and name of every running process on your machine 
(or dev container if you're using the recommended setup). Your output should 
be exactly the same as if you ran ps -eo pid:5,ucmd (except for the PID and 
name of your process). You should start by printing a header, which will be 
PID right-justified with a width of 5 characters, a space, then CMD. To do this, 
your implementation needs to read the /proc directory and its contents. By default, 
the directories should be in the order of ascending pid. Any directory within /proc 
that's a number (e.g. 1) represents a process with that pid. After that you should 
read the /proc/<pid>/status file to get it's name. The file starts with Name: followed 
by a tab character, then the name (anything that isn't a newline character), and 
finally followed by a newline character (not part of the name). For each process 
you should output the pid, right-justified with a width of 5 characters, a space, 
then the name.

You need to check for errors, and properly close all directories and file descriptors. 
You should only need two file descriptors open at any given time. If you fail to open 
a status file, that means the process no longer exists. You should handle this case by 
not printing any information about this now nonexistent process.
*/


void printHeader(){
    printf("  PID CMD\n");
}

int isint(const char *str){
    while(*str){
        if(*str < '0' || *str > '9') {
            return 0;
        }
        str ++;
    }

    return 1;
}

void printProc(const char *fp, const char *pid){
    // create buffer to read into
    char *buf = (char *)malloc(sizeof(char) * 100);

    // create file descriptor and read file
    int fd;
    fd = open(fp, O_RDONLY);

    // check if file opened properly
    if (fd == -1) {
        // Print an error if the file can't be opened
        perror("Error opening file");
        exit(1);
    } else {
        // read to after the first tab
        int i = 0;
        while(read(fd, buf + i, 1) && *(buf + i) != '\t'){
            i ++;
        }

        // read each line from the file
        i = 0;
        while(read(fd, buf + i, 1) && *(buf + i) != '\n'){
            i ++;
        }
        *(buf + i) = '\0';

        printf("%5s %s\n", pid, buf);

        if(close(fd)!=0) { // Don't forget to close the file after opening
            perror("close");
            exit(1);
        }
    }

    free(buf);
}

void readDir(const char *dp){
    // create string for filepath
    char filepath[512];

    // open directory, dp
    DIR *d;
    d = opendir(dp);
    if (d == NULL){
        perror("opendir");
        exit(1);
    }

    // read things in directory
    struct dirent *cur;
    while((cur = readdir(d)) != NULL){
        if (isint(cur->d_name)){
            // make filepath
            snprintf(filepath, sizeof(filepath), "%s/%s/status", dp, cur->d_name);
            // call printProc on file
            printProc(filepath, cur->d_name);
        }
    }

    // Close the directory
    if (closedir(d) != 0) {
        perror("closedir");  // Print error if directory can't be closed
        exit(1);
    }
}

int main() {
    printHeader();
    readDir("/proc");
    return 0;
}
