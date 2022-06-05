  
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

char *prompt;
int sig = 0;

int main() 
{
    char *prompt_temp  = malloc(1024);
    char *tmp_command = malloc(1024);
    char *tmpp = malloc(1024);
    char *ans = malloc(1024);
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
        if(curr_argv > pipes)
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
        if (!access("prev.txt", 0) == 0) 
                {
                    system("touch prev.txt");
                }
        if(piping)
        {
            strcpy(command , argv_s[curr_argv]);
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
                command[strlen(command)] = '\0';
            }
            else{
            command[strlen(command)-1] = '\0';
            }
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
        if (argv[0] == NULL)
            continue;

        //q7 - checks if the first word of the command is quit
        if (! strcmp(argv[0], "quit")) 
        {
            if(argv[1])
            {
                printf("error - bad sbad syntax\n");
                continue;
            }
            for(int k = 0;k<position;k++)
            {
                free(vars[k].key);
                free(vars[k].val);
            }
            for (size_t j = 0; j<10; j++)
            {
                free(argv_s[j]);
            }
                free(prompt_temp);
                free(tmp_command );
                free(tmpp);
                free(ans );

            exit(0);
            return 0;
        }

        /* Does command line end with & */ 
        if (! strcmp(argv[i - 1], "&")) 
        {
            amper = 1;
            argv[i - 1] = NULL;
        }
        else
        {
            amper = 0; 
        } 
            

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
        if (! strcmp(argv[0], "prompt")) 
        {
            if( !argv[1] || strcmp(argv[1], "=") ||  !argv[2] || argv[3])
            {
                printf("error - bad sbad syntax\n");
                if(piping)
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
                continue;
            }
            if(piping)
            {
                curr_argv++;
            }
            memset(prompt_temp , '\0', 1024);
            strcpy(prompt_temp ,argv[2]);
            prompt = prompt_temp;
            continue;
        }

        //q3 is already works with the basic implementation

        //q4 - checks if the first word of the command is echo $?
        if (! strcmp(argv[0], "echo") && ! strcmp(argv[1], "$?") && !argv[2]) 
        {
            if(piping && curr_argv<pipes)
            {
            if (access("prev.txt", 0) == 0) 
                {
                    system("rm prev.txt");
                }
            char st[30];
            if(status == -999)
            {
                sprintf(st,"%s","this is the first command!\n");
            }
            else{
                sprintf(st,"%d",status);
            }
                char tmp_cmd[100] = "echo ";
                strcat(tmp_cmd , st);
                strcat(tmp_cmd , " > prev.txt");
                system(tmp_cmd);
                curr_argv++;
                continue;
            }
           else
            {
             if(status == -999)
            {
                printf("this is the first command!\n");
                continue;
            }
            printf("%d\n" , status);
            continue;
            }
        }
        else if (! strcmp(argv[0], "echo") && argv[1][0]=='$' && !argv[2]) 
        {
            int exist = 0;
            for(int k = 0; k < position; k++)
            {
                if(!strcmp(argv[1] , vars[k].key))
                {
                    memset(ans , '\0', 1024);
                    strcpy(ans,vars[k].val);
                    exist = 1;
                    break;
                }
            }
            if(exist)
            {
                if(piping && curr_argv<pipes)
                {
                    if (access("prev.txt", 0) == 0) 
                    {
                        system("rm prev.txt");
                    }
                    char tmp_cmd[100] = "echo ";
                    strcat(tmp_cmd , ans);
                    strcat(tmp_cmd , " > prev.txt");
                    system(tmp_cmd);
                    curr_argv++;
                    continue;

                }
                printf("%s\n" , ans);
                continue;
            }
            else
            {
                if(piping && curr_argv<pipes)
                {
                    if (access("prev.txt", 0) == 0) 
                    {
                        system("rm prev.txt");
                    }
                                        char tmp_cmd[100] = "echo ";
                    strcat(tmp_cmd , argv[1]);
                    strcat(tmp_cmd , " > prev.txt");
                    system(tmp_cmd);
                    curr_argv++;
                    continue;
                }
                else{
                    printf("%s\n" , argv[1]);
                    continue;
                }
            }
        }
 

        //q5 - checks if the first word of the command is chdir
        if (! strcmp(argv[0], "cd")) 
        {
            if( !argv[1] ||  argv[2])
            {
                printf("error - bad sbad syntax\n");
                if(piping)
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
                continue;
            }
            if(piping)
            {
                curr_argv++;
            }
            chdir(argv[1]);
            continue;
        }

        //q10 - checks if the firs word of the command is $...
        if(argv[0][0] == '$')
        {
            if( strcmp(argv[1], "=") ||  !argv[2] || argv[3])
            {
                printf("error - bad sbad syntax\n");
                if(piping)
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
                continue;
            }
            int exist = 0;
            for(int k = 0; k < position; k++)
            {
                if(!strcmp(argv[0] , vars[k].key))
                {
                    // vars[k]->val = malloc(strlen(argv[2]) +1);
                    strcpy(vars[k].val,argv[2]);
                    exist = 1;
                    break;
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
            }  
            if(piping)
            {
                curr_argv++;
            } 
            continue;
        }
        

        //q11 - checks if the firs word of the command is read...
        if(! strcmp(argv[0], "read")) 
        {
            if( !argv[1] || argv[2])
            {
                printf("error - bad sbad syntax\n");
                if(piping)
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
                continue;
            }
            int exist = 0;
            char key_temp[] = "$";
            char val_temp[1024];
            strcat(key_temp,argv[1]);
            fgets(val_temp, 1024, stdin);
            val_temp[strlen(val_temp)-1] = '\0';
            for(int k = 0; k < position; k++)
            {
                if(!strcmp(key_temp , vars[k].key))
                {
                    strcpy(vars[k].val,val_temp);
                    exist = 1;
                    break;
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
            if(piping)
            {
                curr_argv++;
            }
            continue;
        }

        /* for commands not part of the shell command language */ 
        if(piping && curr_argv < pipes)
        {
            argv[i] = "prev.txt";
            if (fork() == 0) 
            { 

                freopen("prevtmp.txt", "a+", stdout); 
                execvp(argv[0], argv);

            }
            /* parent continues here */
            if (amper == 0)
            {
                retid = wait(&status);
            }
            if(redirect)
            {
                if (access("prev.txt", 0) == 0) 
                {
                    system("rm prev.txt");
                }
                system("touch prev.txt");
            }
            else
            {
                if (access("prev.txt", 0) == 0) 
                {
                    system("rm prev.txt");
                }
                system("cat prevtmp.txt > prev.txt");
                system("rm prevtmp.txt");
            }
            curr_argv++;
        }
        else if((curr_argv == pipes && curr_argv>0) || redirect || redirecterr)
        {
            if(curr_argv == pipes && curr_argv>0)
            {
                argv[i] = "prev.txt";
            }
            if (fork() == 0) 
            { 
                /* redirection of IO ? */
                if (redirect) 
                {
                     if (access(outfile, 0) == 0) 
                    {
                        memset(tmpp, '\0', 1024*sizeof(char));
                        strcat(tmpp , "rm ");
                        strcat(tmpp , outfile);
                        system(tmpp);
                    }
                    freopen(outfile, "a+", stdout); 
                    /* stdout is now redirected */
                }
                else if (redirecterr) 
                {
                 freopen(outfile, "a+", stderr); 
                    /* stderr is now redirected */
                } 
                execvp(argv[0], argv);
            }
            /* parent continues here */
            if (amper == 0)
            {
                retid = wait(&status);
            }
            if(curr_argv == pipes && curr_argv>0)
            {
                curr_argv++;
            }
        }
        else{

                memset(tmp_command , '\0' , 1024);

                int j = 0;
                while(argv[j])
                {
                    strcat(tmp_command , argv[j]);
                    strcat(tmp_command , " ");
                    j++;
                }
                status = system(tmp_command);
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
