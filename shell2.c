#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

void handler_func(int sigg);

char *prompt = "hello";
int sig = 0;

int main() {
char command[1024];
char prev_command[1024];
char *token;
char *outfile;
prompt = "hello";
int i, fd, amper, redirect, retid, status , redirecterr;
char *argv[10];
status = -999;
signal(SIGINT , handler_func);

while (1)
{
    printf("%s: ",prompt);
    fgets(command, 1024, stdin);
    if(sig)
    {
        sig = 0;
        continue;
    }
    if (command[0] == '!' && command[1] == '!')
    {
        strncpy(command , prev_command , 1024);
    }
    strncpy(prev_command , command , 1024);
    command[strlen(command) - 1] = '\0';

    /* parse command line */
    i = 0;
    token = strtok (command," ");
    while (token != NULL)
    {
        argv[i] = token;
        token = strtok (NULL, " ");
        i++;
    }
    argv[i] = NULL;

    /* Is command empty */
    if (argv[0] == NULL)
        continue;

    /* Does command line end with & */ 
    if (! strcmp(argv[i - 1], "&")) {
        amper = 1;
        argv[i - 1] = NULL;
    }
    else 
        amper = 0; 

    //checks if there is redirect in this command
    if (! strcmp(argv[i - 2], ">")) 
    {
        redirect = 1;
        argv[i - 2] = NULL;
        outfile = argv[i - 1];
        printf("out file is : %s\n" , outfile);
    }
    else {redirect = 0; }

    //q1 - checks if there is redirect to stderr in this command
    if (! strcmp(argv[i - 2], "2>")) 
    {
        redirecterr = 1;
        argv[i - 2] = NULL;
        outfile = argv[i - 1];
    }
    else {redirecterr = 0; }

    //q2 - checks if the first word of the command is prompt = ...
    if (! strcmp(argv[0], "prompt") && ! strcmp(argv[1], "=")) 
    {
        prompt = argv[2];
        continue;
    }

    //q3 is already works with the basic implementation

    //q4 - checks if the first word of the command is echo $?
    if (! strcmp(argv[0], "echo") && ! strcmp(argv[1], "$?")) 
    {
        if(status == -999)
        {
            printf("this is the first command!");
            continue;
        }
        printf("%d\n" , status);
        continue;
    }

    //q5 - checks if the first word of the command is chdir
    if (! strcmp(argv[0], "cd")) 
    {
        chdir(argv[1]);
        continue;
    }

    //q7 - checks if the first word of the command is quit
    if (! strcmp(argv[0], "quit")) 
    {
        return 0;
    }

    //q8 - checks if the first word of the command is quit
    if (! strcmp(argv[0], "^C")) 
    {
        continue;
    }

    
        

    /* for commands not part of the shell command language */ 

    if (fork() == 0) 
    { 
        /* redirection of IO ? */
        if (redirect) 
        {
            fd = creat(outfile, 0660); 
            close (STDOUT_FILENO) ; 
            dup(fd); 
            close(fd); 
            /* stdout is now redirected */
        }
        else if (redirecterr) 
        {
            fd = creat(outfile, 0660); 
            close (STDOUT_FILENO) ; 
            dup(fd); 
            close(fd); 
            /* stdout is now redirected */
        } 
        execvp(argv[0], argv);
    }
    /* parent continues here */
    if (amper == 0)
        retid = wait(&status);
}
}
void handler_func(int sigg)
{
    //printf("%s\n" , prompt);
    //signal(SIGINT , handler_func);
    printf("\nyou typed Control-C!\n%s: " , prompt);
    sig = 1;
}
