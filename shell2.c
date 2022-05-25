#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <signal.h>
#include <stdbool.h>
#define _SVID_SOURCE 1

typedef struct pair{
    char *key;
    char *val;
}pair;

struct pair vars[100];

int position = 0;

// int insert(char *n_key , char *n_val , int pos)
// {
//     printf("pos is : %d\n" , pos);
//     for(int k = 0; k < pos; k++)
//     {
//         if(!strcmp(n_key , vars[k].key))
//         {
//             vars[k].val = n_val;
//             return 0;
//         }
//     }
//     pair tmp_pair;
//     strcpy(tmp_pair.key,n_key);
//     strcpy(tmp_pair.val,n_val);
//     vars[pos] = tmp_pair;
//     //printf("value : %s inserted into key : %s\n" , vars[position].val , vars[position].key);
//     return 1;
// }


// char* get(char *n_key , int pos)
// {
//     for(int k = 0; k < pos; k++)
//     {
//         printf("iter : %d , key : %s , val ; %s\n" , k ,vars[k].key,vars[k].val);
//         if(!strcmp(n_key , vars[k].key))
//         {
//             return vars[k].val;
//         }
//     }
//     return "";
// }

void handler_func(int sigg);

struct termios termios_save;

void reset_the_terminal(void)
{
tcsetattr(0, 0, &termios_save );
}

sig_atomic_t the_flag = 0;

char *prompt = "hello";
int sig = 0;

int main() {
char command[1024];
char prev_command[1024];
char *token;
char *outfile;
prompt = "hello";
int i, fd, amper, redirect, retid, status , redirecterr , position;
char *argv[10];
status = -999;
position = 0;


int rc;
int ch;
struct termios termios_new;

rc = tcgetattr(0, &termios_save );
if (rc) {perror("tcgetattr"); exit(1); }

rc = atexit(reset_the_terminal);
if (rc) {perror("atexit"); exit(1); }

termios_new = termios_save;
termios_new.c_lflag &= ~ECHOCTL;
rc = tcsetattr(0, 0, &termios_new );
if (rc) {perror("tcsetattr"); exit(1); }


signal(SIGINT , handler_func);

while (1)
{
    printf("%s: ",prompt);
    fgets(command, 1024, stdin);
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

    //q7 - checks if the first word of the command is quit
    if (! strcmp(argv[0], "quit")) 
    {
        return 0;
    }

    /* Does command line end with & */ 
    if (! strcmp(argv[i - 1], "&")) 
    {
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

    //q10 - checks if the firs word of the command is $...
    if(argv[0][0] == '$')
    {
        int exist = 0;
        for(int k = 0; k < position; k++)
        {
            if(!strcmp(argv[0] , vars[k].key))
            {
                // vars[k]->val = malloc(strlen(argv[2]) +1);
                strcpy(vars[k].val,argv[2]);
                exist = 1;
                continue;
            }
        }
        if(!exist)
        {
            pair *tmp_pair;
            vars[position].key = malloc(strlen(argv[0]) +1);
            vars[position].val = malloc(strlen(argv[2]) +1);
            strcpy(vars[position].key,argv[0]);
            strcpy(vars[position].val,argv[2]);
            printf("vars : ");
            position ++;
            for(int k = 0; k <position;k++)
            {
                printf("pos : %d , key : %s , val :%s\n" , k , vars[k].key , vars[k].val);
            }
        }   
        continue;
    }
    if (! strcmp(argv[0], "echo") && argv[1][0]=='$') 
    {
        int exist = 0;
        char *ans;
        for(int k = 0; k < position; k++)
        {
            //printf("iter : %d , key : %s , val ; %s\n" , k ,vars[k].key,vars[k].val);
            if(!strcmp(argv[1] , vars[k].key))
            {
                ans = malloc(strlen(vars[k].val)+1);
                strcpy(ans,vars[k].val);
                exist = 1;
            }
        }
        if(exist)
        {
            printf("%s\n" , ans);
            continue;
        }
        //printf("the returned value should be 'pitma' , and is : %s\n" , ans);
        // if(strcmp(ans , ""))
        // {
        //     printf("%s\n" , ans);
        //     continue;
        // }
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
            close (STDERR_FILENO) ; 
            dup2(fd , STDERR_FILENO); 
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
    sig = 1;
    signal(SIGINT , handler_func);
    printf("\nyou typed Control-C!\n");
    printf("%s :" , prompt);
    fflush(stdout);
}
