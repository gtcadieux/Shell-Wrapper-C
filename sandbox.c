// sandbox.c
// Gabriel Cadieux
// Last Edited: 11/30/23
// Shell program that mimics a linux terminal, handles internal commands: exit, cd, and jobs. 
// Otherwise runs executables with fork
// Note: uses vector.h, needs to be compiled with marz's vector library -lcvector

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "vector.h"
#include <sys/resource.h>
#include <sys/wait.h>
#include <getopt.h>
#include <fcntl.h>

// Changes Directorty using chdir
void changeDirectory(char *dir) 
{
    if ((strcmp(dir, "~") == 0) || (strcmp(dir, ".") == 0)) 
    {
        strcpy(dir, getenv("HOME"));
        if(chdir(dir) != 0) 
        {
            perror("cd");
        }
        return;
    }
    if (chdir(dir) != 0) 
    {
        perror("cd");
    }
    return;
}

// Swaps an environment variable for its expanded counterpart
char *swapEnvVar(char *envVar) 
{
    int dollarIndex = 0;
    int endIndex = 0;
    // Find the start (0) and end (first space or / or endline or null term) of environment variable
    for (int i = 0; i < (strlen(envVar) + 1); i++) 
    {
        if ((envVar[i] == '/') || (envVar[i] == '\n') || (envVar[i] == ' ') || (envVar[i] == '\0')) 
        {
            endIndex = i;
            break;
        }
        else {
        }
    }

    // Extract the environmental variable, get its expansion and return it
    char cutout[endIndex];
    strncpy(cutout, envVar + 1, endIndex - 1);
    cutout[endIndex - 1] = '\0';
    char envDir[256];
    strncpy(envDir, getenv(cutout), 256);
    strncat(envDir, envVar + endIndex, strlen(envVar));
    char *newptr = calloc(strlen(envDir) + 1, sizeof(char));
    strncpy(newptr, envDir, strlen(envDir));
    newptr[strlen(envDir)] = '\0';
    return newptr;
}

int main(int argc, char **argv) 
{

    // OPT SWITCHES
    int opt;
    int p_switch = 256;
    int d_switch = 1 << 30;
    int s_switch = 1 << 30;
    int n_switch = 256;
    int f_switch = 1 << 30;
    int t_switch = 1 << 30;

    while ((opt = getopt(argc, argv, "p:d:s:n:f:t:") != -1)) 
    {
        switch (opt) 
            {   
                case 'p':
                    if (sscanf(optarg, "%d", &p_switch) != 1) 
                    {
                        fprintf(stderr, "Error with -p switch '%s' is invalid.\n", optarg);
                    }
                break;
                case 'd':
                    if (sscanf(optarg, "%d", &p_switch) != 1) 
                    {
                        fprintf(stderr, "Error with -p switch '%s' is invalid.\n", optarg);
                    }
                break;
                case 's':
                    if (sscanf(optarg, "%d", &p_switch) != 1) 
                    {
                        fprintf(stderr, "Error with -p switch '%s' is invalid.\n", optarg);
                    }
                break;
                case 'n':
                    if (sscanf(optarg, "%d", &p_switch) != 1) 
                    {
                        fprintf(stderr, "Error with -p switch '%s' is invalid.\n", optarg);
                    }
                break;
                case 'f':
                    if (sscanf(optarg, "%d", &p_switch) != 1) 
                    {
                        fprintf(stderr, "Error with -p switch '%s' is invalid.\n", optarg);
                    }
                break;
                case 't':
                    if (sscanf(optarg, "%d", &p_switch) != 1) 
                    {
                        fprintf(stderr, "Error with -p switch '%s' is invalid.\n", optarg);
                    }
                break;
                default:
                    fprintf(stderr, "Usage: %s [-o value]\n", argv[0]);
                break;
            }
    }

    // Main Variables and Vectors
    char cmdbuf[256];
    char cwd[256];
    char homeDir[256];
    strncpy(homeDir, getenv("HOME"), 256);
    int count = 0;
    void *inputVec = vector_new();
    void *pids = vector_new();
    void *backgroundProcs = vector_new();

    // Bool variables for redirection
    int read = 0;
    int write = 0;
    int append = 0;
    char readFile[256];
    char writeFile[256];
    char appendFile[256];

    while(1) 
    {
        // Get current working directory
        getcwd(cwd, sizeof(cwd));
        for (int i = 0; i < strlen(homeDir); i++) 
        {
            if (homeDir[i] == cwd[i]) 
            {
                count++;
            }
            else 
            {
                break;
            }
        }

        // Get home directory
        if (count == (strlen(homeDir))) 
        {
            char tmpstr[256];
            strcpy(tmpstr, &cwd[strlen(homeDir)]);
            memset(cwd, 0, sizeof(cwd));
            cwd[0] = '~';
            strcat(cwd, tmpstr);
        }
        
        // Print gcadieux@sandbox:~/CS360/lab8 for example and wait for input
        printf("%s@sandbox:%s> ", getenv("USER"), cwd);
        fgets(cmdbuf, 256, stdin);

        // Variables for chopping up args and finding environment variables
        int checkCount = 0;
        int charCount = 0;
        int numArgs = 0;
        int enVar = 0;
        int greaterCount = 0;
        int lessCount = 0;
        int isFile = 0;

        // Chops up args and stores then into a vector
        while (cmdbuf[charCount] != '\0') 
        {
            // If we see $ it is an environmental variable
            if (cmdbuf[charCount] == '$') 
            {
                enVar = 1;
            }
            if (cmdbuf[charCount] == '>') 
            {
                greaterCount++;
            }
            if (cmdbuf[charCount] == '<') 
            {
                lessCount++;
            }

            // When we see a space or endline, that is the end of that arg, store it in vector
            if ((cmdbuf[charCount] == ' ') || (cmdbuf[charCount] == '\n')) 
            {
                // Make a pointer with enough room for argument plus null terminator
                char tmpptr[checkCount + 1];
                strncpy(tmpptr, cmdbuf + (charCount - checkCount), checkCount);
                tmpptr[checkCount] = '\0';

                if (greaterCount == 2) 
                {
                    append = 1;
                    isFile = 1;
                    strncpy(appendFile, cmdbuf + ((charCount - checkCount) + 2), checkCount - 2);
                    appendFile[checkCount - 2] = '\0';
                }
                if (greaterCount == 1) 
                {
                    write = 1;
                    isFile = 1;
                    strncpy(writeFile, cmdbuf + ((charCount - checkCount) + 1), checkCount - 1);
                    writeFile[checkCount - 1] = '\0';
                }
                if (lessCount == 1) 
                {
                    read = 1;
                    isFile = 1;
                    strncpy(readFile, cmdbuf + ((charCount - checkCount) + 1), checkCount - 1);
                    readFile[checkCount - 1] = '\0';
                }

                // If this argument is a file, do nothing
                if (isFile) 
                {
                    //Do nothing;
                }
                // If it is an environmental variable, swap it before storing in vector
                else if (enVar == 1) 
                {
                    // printf("Swapping Envar\n");
                    char *newptr = swapEnvVar(tmpptr);
                    vector_push_ptr(inputVec, newptr);
                }
                // else, store it in the vector
                else 
                {
                    char *newptr = calloc(strlen(tmpptr) + 1, sizeof(char));
                    strncpy(newptr, tmpptr, strlen(tmpptr));
                    newptr[strlen(tmpptr)] = '\0';
                    vector_push_ptr(inputVec, newptr);
                }
                
                lessCount = 0;
                greaterCount = 0;
                checkCount = 0;
                enVar = 0;
                numArgs++;
            }
            // checkCount counts how many regular chars we've seen, essentially measuring how long arguments are
            else 
            {
                checkCount++;
            }
            charCount++;
        }

        // Variables to convert vector into char ** and check for & to mark background processes
        int64_t v;
        int backgroundProc = 0;
        char **args = calloc(vector_size(inputVec), sizeof(char *));
        for (int i = 0; i < vector_size(inputVec); i++) 
        {
            // If it is a background process, we want to store the command into a vector, do not pass & to args
            vector_get(inputVec, i, &v);
            if (strcmp((char *)v, "&") == 0) 
            {
                backgroundProc = 1;
                char *cmd = calloc(strlen(cmdbuf) + 1, sizeof(char));
                strncpy(cmd, cmdbuf, (strlen(cmdbuf)));
                cmd[strlen(cmdbuf)] = '\0';
                cmd[strlen(cmdbuf) - 1] = '\0';
                cmd[strlen(cmdbuf) - 2] = '\0';
                vector_push_ptr(backgroundProcs, cmd);
            }
            // Else, store argument in args
            else 
            {
                args[i] = (char *)v;
            }
        }

        // EXIT COMMAND - Frees allocated memory and returns.
        if (strcmp(args[0], "exit") == 0) 
        {
            for (int i = 0; i < vector_size(inputVec); i++) 
            {
                free(args[i]);
            }
            free(args);
            return 0;
        }

        // CHANGE DIRECTORY COMMAND - Changes the current working directory to specifed directory
        else if (strcmp(args[0], "cd") == 0) 
        {
            if (numArgs < 2) 
            {
                changeDirectory(homeDir);
            }
            else 
            {
                for (int i = 1; i < numArgs; i++) 
                {
                    changeDirectory(args[i]);
                }
            }
        }

        // LIST ALL JOBS - List all background processes' pids and commands
        else if (strcmp(args[0], "jobs") == 0)
        {
            int64_t proc;
            for (int i = 0; i < vector_size(backgroundProcs); i++) 
            {
                vector_get(pids, i, &proc);
                proc = (pid_t)proc;
                if (waitpid(proc, NULL, WNOHANG) == proc) 
                {
                    vector_remove(backgroundProcs, i);
                    vector_remove(pids, i);
                }
            }
            int64_t job;
            int64_t pid;
            printf("%d jobs.\n", vector_size(backgroundProcs));
            for (int i = 0; i < vector_size(backgroundProcs); i++) 
            {
                vector_get(backgroundProcs, i, &job);
                vector_get(pids, i, &pid);
                pid = (pid_t)pid;
                char * ptr = (char *)job;
                printf("    %d  - %s\n", pid, ptr);
            }
        }

        // ATTEMPT TO RUN EXECUTABLE - Will attempt to fork and run args[0] if it is not an internal command above
        else 
        {
            pid_t pid = fork();
            if (pid < 0) 
            {
                perror("fork");
            }

            // CHILD
            else if (pid == 0) 
            {
                if (read) 
                {
                    int fd = open(readFile, O_RDONLY, 0600);
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }
                if (write) 
                {
                    int fd = open(writeFile, O_CREAT | O_WRONLY, 0600);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
                if (append) 
                {
                    int fd = open(appendFile, O_CREAT | O_WRONLY | O_APPEND, 0600);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
                // Impose resource limits on child and execute
                setrlimit(RLIMIT_NPROC, &(struct rlimit){p_switch, p_switch});
                setrlimit(RLIMIT_DATA, &(struct rlimit){d_switch, d_switch});
                setrlimit(RLIMIT_STACK, &(struct rlimit){s_switch, s_switch});
                setrlimit(RLIMIT_NOFILE, &(struct rlimit){n_switch, n_switch});
                setrlimit(RLIMIT_FSIZE, &(struct rlimit){f_switch, f_switch});
                setrlimit(RLIMIT_CPU, &(struct rlimit){t_switch, t_switch});
                execvp(args[0], args);
                perror("execvp");
                exit(EXIT_FAILURE);
            }

            // PARENT
            else 
            {
                // If process was specified with '&', store the process id
                if (backgroundProc) 
                {
                    vector_push(pids, pid);
                }
                // Otherwise, wait for the process to finish
                else 
                {
                    waitpid(pid, NULL, 0);
                }
            }
        }

        // Reset variables to read more input
        count = 0;
        charCount = 0;
        vector_clear(inputVec);
        read = 0;
        write = 0;
        append = 0;
        greaterCount = 0;
        lessCount = 0;
        isFile = 0;
        readFile[0] = '\0';
        writeFile[0] = '\0';
        appendFile[0] = '\0';
    }
}