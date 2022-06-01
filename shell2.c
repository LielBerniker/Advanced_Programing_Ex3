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

void handler_func(int sigg);
int pipe_handler(char *cmd , char *prev_cmd);

struct termios termios_save;

void reset_the_terminal(void)
{
    tcsetattr(0, 0, &termios_save );
}

sig_atomic_t the_flag = 0;

char *prompt = "hello";
int sig = 0;

int main() 
{
    char command[1024];
    char prev_command[1024];
    char *token;
    char *outfile;
    prompt = "hello";
    int i, fd, amper, redirect, retid, status , redirecterr , position , piping;
    char *argv[1000];
    char *argv_s[10];
    int curr_argv , pipes;
    status = -999;
    position = 0;
    int tmp = 0;


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
    piping = 0;
    pipes = 0;
    curr_argv = 0;
    for (size_t i = 0; i < 10; i++)
    {
       argv_s[i] = malloc(1024);
    }
    
    while (1)
    {
        if(piping)
        {
            strcpy(command , argv_s[curr_argv]);
            strcat(command , " ");
            strcat(command , "prev.txt");
        }
        else
        {
            //get command from user 
            printf("%s: ",prompt);
            fflush(stdin);
            fgets(command, 1024, stdin);
            if (command[0] == '!' && command[1] == '!')
            {
                strncpy(command , prev_command , 1024);
            }
            command[strlen(command) - 1] = '\0';
            //checks if there is | in command , and counts the pipes
            for (int k = 0; k < strlen(command);k++)
            {
                if (command[k] == '|')
                {
                    pipes++;
                    if(!piping)
                    {
                        piping = 1;
                    }
                } 
            }
            //if there is | in the new command - get into piping stage ,  and initalize the variables.
            if (piping)
            {
                int last = 0;
                for(int k = 0;k<strlen(command);k++)
                {
                    if(command[k] == '|')
                    {
                        strncpy(argv_s[curr_argv] , (command+last) , k-(last+1));
                        last = k+2;
                        curr_argv++;
                    }
                }
                strcpy(argv_s[curr_argv] , (command+last));
                curr_argv = 0;
                strcpy(command , argv_s[curr_argv]);
                
            }
        }
        strncpy(prev_command , command , 1024);
        // printf("the command are : %s\n" , command);
        /* parse command line */
        i = 0;
        token = strtok (command," ");
        while (token != NULL)
        {
            //printf("token nember %d is : %s\n",i,token);
            argv[i] = token;
            token = strtok (NULL, " ");
            i++;
        }
        argv[i] = NULL;
        // for(int k=0;k<i;k++)
        // {
        //     printf("argv[%d] is : %s\n" , k , argv[k]);
        // }
        /* Is command empty */
        if (argv[0] == NULL)
            continue;

        //q7 - checks if the first word of the command is quit
        if (! strcmp(argv[0], "quit")) 
        {
            for(int k = 0;k<position;k++)
            {
                free(vars[k].key);
                free(vars[k].val);
            }
          
                for (size_t j = 0; j<10; j++)
                {
                    free(argv_s[j]);
                }
            
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
        if(i>=2)
        {
        if (! strcmp(argv[i - 2], ">")) 
        {
            redirect = 1;
            argv[i - 2] = NULL;
            outfile = argv[i - 1];
        }

        //q1 - checks if there is redirect to stderr in this command
        else if (! strcmp(argv[i - 2], "2>")) 
        {
            redirecterr = 1;
            argv[i - 2] = NULL;
            outfile = argv[i - 1];
        }
        else 
        {
            redirecterr = 0; 
            redirect = 0;
        }
        }
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
                position ++;
                // for(int k = 0; k <position;k++)
                // {
                //     printf("pos : %d , key : %s , val :%s\n" , k , vars[k].key , vars[k].val);
                // }
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
                printf("%s" , ans);
                continue;
            }
        }

        //q11 - checks if the firs word of the command is read...
        if(! strcmp(argv[0], "read")) 
        {
            // printf("\n");
            int exist = 0;
            char key_temp[] = "$";
            char val_temp[1024];
            strcat(key_temp,argv[1]);
            fgets(val_temp, 1024, stdin);
            for(int k = 0; k < position; k++)
            {
                if(!strcmp(key_temp , vars[k].key))
                {
                    strcpy(vars[k].val,val_temp);
                    exist = 1;
                    continue;
                }
            }
            if(!exist)
            {
                vars[position].key = malloc(strlen(key_temp) +1);
                vars[position].val = malloc(strlen(val_temp) +1);
                strcpy(vars[position].key,key_temp);
                strcpy(vars[position].val,val_temp);
                position ++;
            }   
            continue;
        }

        /* for commands not part of the shell command language */ 

        // for(int k=0;k<=i;k++)
        // {
        //     printf("%d : %s=\n" ,k , argv[k]);
        // }
        if(piping && curr_argv < pipes)
        {
            if (fork() == 0) 
            { 
                /* redirection of IO ? */
                if (redirect) 
                {
                    // fd = creat(outfile, 0660); 
                    // close (STDOUT_FILENO) ; 
                    // dup(fd); 
                    // close(fd); 
                    freopen(outfile, "a+", stdout); 
                    /* stdout is now redirected */
                }
                else if (redirecterr) 
                {
                    fd = creat(outfile, 0660); 
                    close (STDERR_FILENO) ; 
                    dup2(fd , STDERR_FILENO); 
                    close(fd);

                    int fd2 = creat("prev.txt", 0660); 
                    close (STDOUT_FILENO) ; 
                    dup(fd2); 
                    close(fd2); 
                }
                else
                {
                    freopen("prevtmp.txt", "a+", stdout); 
                }
                //need to redirect output to the prev_command string
                execvp(argv[0], argv);
            }
            /* parent continues here */
            if (amper == 0)
            {
                retid = wait(&status);
            }
            if (access("prev.txt", 0) == 0) 
            {
                system("rm prev.txt");
            }
            system("cat prevtmp.txt > prev.txt");
            system("rm prevtmp.txt");


            curr_argv++;
            //continue;
        }
        else
        {
            if (fork() == 0) 
            { 
                /* redirection of IO ? */
                if (redirect) 
                {
                    // fd = creat(outfile, 0660); 
                    // close (STDOUT_FILENO) ; 
                    // dup(fd); 
                    // close(fd); 
                    freopen(outfile, "a+", stdout); 
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
            {
                retid = wait(&status);
            }
            if(curr_argv == pipes)
            {
                for(int k = 0;k<10;k++)
                {
                     memset(argv_s[k], '\0', 1024*sizeof(char));
                }
                curr_argv = 0;
                piping = 0;
                pipes = 0;
                if (access("prev.txt", 0) == 0) 
                {
                    system("rm prev.txt");
                }
                if (access("prevtmp.txt", 0) == 0) 
                {
                    system("rm prevtmp.txt");
                }
            }
        }
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