/* 
   xssh.c for Operating System Lab
   October 5 2011 Zheng Luo
   Modified by 罗铮 on 03/19/14.
 */

#include   <sys/types.h>
#include   <sys/stat.h>
#include   <sys/wait.h>
#include   <stdio.h>
#include   <string.h>
#include   <unistd.h>
#include   <stdlib.h>
#include   <signal.h>
#include   <errno.h>
#include   <stdlib.h>
#include   <fcntl.h>

#define   MAXTOKEN   20 /* max number of command tokens */
#define   BUFFERSIZE   256 /* input buffer size */

struct variables {
    char *name;
    char *value;
} words [MAXTOKEN];/* Storing variables */

/* implementation of chdir */
void do_cd(char *argv[]); 
/* implementation of setting variables */
void do_set(char *argv[]); 
/* implementation of seting showing variables*/
void do_show(char *argv[]);
/* implementation of seting environment variables */
void do_set_environment(char *argv[]);
/* implementation of unseting environment variables */
void do_unset_environment(char *argv[]);
/* implementation of handler afer receiving the sigals */
void signal_handler(int sig);
/* search value */
int  search_variable(char *argv1,char *argv2);
/* implementation of execute external command or applications */
void execute_foreground(char *argv[]);
/* implementation of execute external command or applications
   containing pipes */
void execute_pipe(char *argv[]);
/* implementation of execute background commands */
int  execute_background(char *argv[]);
/* implementation of commands wait I */
void waitprocess(char *argv[]);
int sigignore(int sig);

int n = 0;/* parse index for set value */
int i,j,k,b ;/* parse index*/
int condition = 0;
int childpid = 0;
FILE *fp;/* file pointer */
int fd, id;
int valueReturn = 0;
int re_flag = 0;
int number = 0;
/*indicated whether there is & in the command*/
int backgroundflag = 0;
/*indicated the process id of the last background command*/
pid_t pidb = 0;
/*indicated the background status*/
int bgstatus = 0;

int main()
{
    char cmd[BUFFERSIZE]; /*command input buffer*/
    /* array of pointers to command line tokens */
    char *cmd_arg[MAXTOKEN]; 
    /* command line token delimiters */
    char *delimiters = " \t\n";
    int  cmdlen; /* parse index */
    int status = 0;/* return value of findvalue */
    int parentpid = 0;
    /*indicated whether there is | in the command*/
    int pipeflag = 0;

    /* getting the ctrl+c signals*/
    struct sigaction act, oldact;
    act.sa_handler = signal_handler;
    sigaction(SIGINT, &act, &oldact);
    while(1)
    {
        printf(">> ");
        fgets(cmd, BUFFERSIZE, stdin);
        cmdlen=strlen(cmd);
        cmdlen--;
        cmd[cmdlen]='\0';
        if (cmd[0] == '#'|| cmd[0] == '\n'|| cmd[0] == '\0')
        {
            continue;
        }
        cmd_arg[0] = strtok(cmd, delimiters);
        for (i = 1; i < MAXTOKEN; i++)
        {
            cmd_arg[i] = strtok(NULL, delimiters);
            if (cmd_arg[i] == NULL)
            {
                number = i;
                break;
            }
        }
        /*searching for $*/
        for (i = 0; i < MAXTOKEN; i++)
        {
            if (cmd_arg[i] == NULL)
            {
                break;
            }
            else
            {
                if(*cmd_arg[i] == '$')/*searching for $*/
                {
                    /*searching for $$*/
                    if(strcmp(cmd_arg[i],"$$")==0)
                    {
                        parentpid =  getpid();
                        printf("The xssh PID is: %d \n", parentpid);
                        status = -1;
                    }
                    /*searching for $?*/
                    else if(strcmp(cmd_arg[i],"$?")==0)
                    {
                        printf("Decimal value returned by the "
                                "last foreground command is: %d \n", 
                                valueReturn);
                        status = -1;
                    }
                    /*searching for $!*/
                    else if(strcmp(cmd_arg[i],"$!")==0)
                    {
                        if(pidb == 0)
                        {
                            printf("No background processes");
                        }
                        else
                        {
                            printf("The last background process"
                                    " id : %d \n", pidb);
                            status = -1;
                        }
                    }
                    else
                    {
                        status = search_variable((cmd_arg[i]+1),
                                cmd_arg[i]);
                    }
                }
            }
        }
        /* no value found then continue or 
           something int the command line*/
        if (status == -1)
        {
            status = 0;
            continue;
        }
        /* exit I */
        if(strcmp(cmd_arg[0],"exit")==0)
        {
            break;
        }
        /* chdir */
        if(strcmp(cmd_arg[0],"chdir")==0)
        {
            do_cd(cmd_arg);
            continue;
        }
        /* set variables */
        if(strcmp(cmd_arg[0],"set")==0)
        {
            do_set(cmd_arg);
            continue;
        }
        /* show variables */
        if(strcmp(cmd_arg[0],"show")==0)
        {
            do_show(cmd_arg);
            continue;
        }
        /* setting evironment variables */
        if(strcmp(cmd_arg[0],"export")==0)
        {
            do_set_environment(cmd_arg);
            continue;
        }
        if(strcmp(cmd_arg[0],"unexport")==0)
        {
            do_unset_environment(cmd_arg);
            continue;
        }
        /* wait process to terminate */
        if(strcmp(cmd_arg[0],"wait")==0)
        {
            waitprocess(cmd_arg);
            continue;
        }
        /* background command or applicaitons*/
        for (i = 0; i < number; i++)
        {
            if(strcmp(cmd_arg[i],"&")== 0)
            {
                backgroundflag = 1;
                execute_background(cmd_arg);
            }
        }
        if(backgroundflag == 1)
        {
            backgroundflag = 0;
            continue;
        }
        /* exteranl command or applicaitons*/
        for (i = 0; i < number; i++)
        {
            if(strcmp(cmd_arg[i],"|")== 0)
            {
                pipeflag = 1;
            }
        }
        if((pipeflag == 0) && (backgroundflag ==0))
        {
            /* execute the no pipe fuction  */
            execute_foreground(cmd_arg);
        }
        if((pipeflag == 1) && (backgroundflag ==0))
        {
            pipeflag = 0;
            execute_pipe(cmd_arg);
        }
    } /*   while   */
    if(cmd_arg[0]==NULL)
    {
        return 0;
    }
    else
    {
        return atoi(cmd_arg[0]);
    }
} /*   main   */

/* implementation of chdir */
void do_cd(char *argv[])
{
    if(argv[1]!=NULL)
    {
        if(chdir(argv[1])<0)
            switch(errno)
            {
                case ENOENT:
                    printf("DIRECTORY NOT FOUND\n");
                    break;
                case ENOTDIR:
                    printf("NOT A DIRECTORY NAME\n");
                    break;
                case EACCES:
                    printf("YOU DO NOT HAVE RIGHT TO ACCESS\n");
                    break;
                default:
                    printf("SOME ERROR HAPPENED IN CHDIR\n");
            }
    }
}

/* implementation of setting variables */
void do_set(char *argv[])
{
    if((argv[1]!=NULL) && (argv[2]!=NULL) && (argv[3]==NULL))
    {
        words[n].name = (char *)malloc(10 * sizeof(char));
        words[n].value = (char *)malloc(10 * sizeof(char));
        strcpy(words[n].name,argv[1]);
        strcpy(words[n].value,argv[2]);
        n++;
    }
    else
    {
        printf("PLEASE INPUT TWO VARIABLES\n");
    }
}

/* implementation of showing variables */
void do_show(char *argv[])
{
    /* the flag of telling whether there exists that variable */
    int flag = 0;
    if((argv[1]!=NULL) && (argv[2]==NULL))
    {
        for (k = 0; k < n ; k++)
        {
            if(strcmp((words[k].name),argv[1])==0)
            {
                printf("The value of %s is : %s\n",
                        words[k].name,words[k].value);
                flag = 1;
            }
        }
        if(flag == 0)
        {
            printf("NO THAT VAVIRABLE CALLED %s \n",argv[1]);
        }
    }
    else
    {
        printf("PLEASE INPUT CORRECT COMMAND\n");
    }
}

/* implementation of setting variables */
void do_set_environment(char *argv[])
{
    /* the flag of telling whether there exists that variable */
    int flag = 0;
    if((argv[1]!=NULL) && (argv[2]==NULL))
    {
        for (k = 0; k < n ; k++)
        {
            if(strcmp((words[k].name),argv[1])==0)
            {
                setenv(argv[1], words[k].value, 0);
                flag = 1;
            }
        }
    }
    if(flag == 0)
    {
        printf("SETTING ENVIRONMENT VARIABLE FAILED\n");
    }
}

/* implementation of unsetting variables */
void do_unset_environment(char *argv[])
{
    /* the flag of telling whether there exists that variable */
    int flag = 0;
    if((argv[1]!=NULL) && (argv[2]==NULL))
    {
        for (k = 0; k < n ; k++)
        {
            if(strcmp((words[k].name),argv[1])==0)
            {
                unsetenv(argv[1]);
                flag = 1;
            }
        }
    }
    if(flag == 0)
    {
        printf("UNSETTING ENVIRONMENT VARIABLE FAILED\n");
    }
}

/* implementation of handler afer receiving the sigals */
void signal_handler(int sig)
{
    /* foreground process termination*/
    if(childpid != 0)
    {
        int termistate = kill(childpid,sig);
        if(termistate == 0)
        {
            printf(" Foreground Process Terminated"
                    " and pid is /:%d\n",childpid);
            //printf("termistate %d\n",termistate);
        }
        else
        {
            printf("  Termination Foreground Processes "
                    "Failed childpid is %d\n",childpid);
            //printf("termistate %d\n",termistate);
        }
    }
    else
    {
        printf("  No Foreground Process\n");
    }
}

/* implementation of search value */
int search_variable(char *argv1,char *argv2)
{
    /* the flag of telling whether there exists that variable */
    int flag = 0;
    for (k = 0; k < n ; k++)
    {
        if(strcmp((words[k].name),argv1)==0)
        {
            strcpy(argv2,words[k].value);
            flag = 1;
        }
    }
    if(flag == 0)
    {
        printf("ERRORS\n");
        return -1;
    }
    return 0;
}

/* implementation of execute external command or applications */
void execute_foreground(char *argv[])
{
    pid_t pid;
    pid=fork();
    if(pid<0)
    {
        printf("SOME ERROR HAPPENED IN FORK\n");
        exit(2);
    }
    else if(pid==0)
    {
        /*
           printf("The parentpid is %ld\n",
           (long int)getppid());
           printf("The childpid is %ld\n",
           (long int)getpid());
         */

        /* redirecting stdout */
        for (i = 0; i < number; i++)
        {
            if(strcmp(argv[i],">")== 0)
            {
                fp = fopen(argv[i+1], "w");
                fd = fileno(fp);
                dup2(fd, STDOUT_FILENO);
            }
            if(strcmp(argv[i],"<")== 0)
            {
                fp = fopen(argv[i+1], "r");
                if (fp == NULL)
                {
                    printf("read error.\n");
                }
                fd = fileno(fp);
                dup2(fd, STDIN_FILENO);
            }
            if( (strcmp(argv[i],">")== 0) ||
                    (strcmp(argv[i],"<")== 0) )
            {
                argv[i]= NULL;
            }
        }
        condition = execvp(argv[0],argv);
        if(condition < 0)
        {
            switch(errno)
            {
                case ENOENT:
                    printf("COMMAND OR FILENAME NOT FOUND\n");
                    break;
                case EACCES:
                    printf("YOU DO NOT HAVE RIGHT TO ACCESS\n");
                    break;
                default:
                    printf("SOME ERROR HAPPENED IN EXEC\n");
            }
        }
        exit(3);
    }
    else
    {
        childpid = waitpid(pid,&valueReturn,0);
    }
}

/* implementation of execute external 
   command or applications containing pipes */
void execute_pipe(char *argv[])
{
    int pfds[10][2];
    pid_t pid;
    char *argp[MAXTOKEN][MAXTOKEN];
    int n = 0;
    int m = 0;
    /*the flag indicate that whether there is redirection*/
    int redpflagout = 0;
    /*the flag indicate that whether there is redirection*/
    int redpflagin = 0;
    /* find if there is redirection in the end*/

    /*store commands in the 2D array*/
    for (i =0; argv[i]!= NULL; i++)
    {
        if(strcmp(argv[i],"|")== 0)
        {
            argp[m][n] = NULL;
            m++;
            n = 0;
        }
        else
        {
            argp[m][n] = argv[i];
            n++;
        }
    }
    for (i = 0;i<10; i++)
    {
        if( pipe( pfds[i] )!=0 )
        {
            printf("errors\n");
        }
    }
    for (i = 0;i <=m; i++)
    {
        pid=fork();
        if( pid == 0)
        {
            if(i==0)
            {
                for (j = 0; argp[0][j]!=NULL; j++)
                {
                    if(strcmp(argp[0][j],"<")== 0)
                    {
                        argp[0][j]= NULL;
                        redpflagin = 1;
                        fp = fopen(argp[0][j+1], "r");
                    }

                }
                if(redpflagin ==1)
                {
                    if (fp == NULL)
                    {
                        printf("read error.\n");
                    }
                    fd = fileno(fp);
                    dup2(fd, STDIN_FILENO);
                    redpflagin = 0;
                }
                close(pfds[0][0]);
                dup2(pfds[0][1], 1);
                close(pfds[0][1]);
            }
            else if (i==m)
            {
                for (j = 0; argp[i][j]!=NULL; j++)
                {
                    if(strcmp(argp[i][j],">")== 0)
                    {
                        argp[i][j]= NULL;
                        redpflagout = 1;
                        fp = fopen(argp[i][j+1], "w");
                    }

                }
                if(redpflagout == 1)
                {
                    fd = fileno(fp);
                    dup2(fd,1);
                    redpflagout =0;
                }
                close(pfds[i-1][1]);
                dup2(pfds[i-1][0], 0);
                close(pfds[i-1][0]);
            }
            else
            {
                close(pfds[i-1][1]);
                dup2(pfds[i-1][0], 0);
                close(pfds[i-1][0]);
                close(pfds[i][0]);
                dup2(pfds[i][1], 1);
                close(pfds[i][1]);
            }
            execvp(argp[i][0],argp[i]);
            exit(0);
        }
        else
        {
            close(pfds[i][1]);
            waitpid(pid,NULL,0);
        }
    }
}

/* implementation of execute background commands */
int execute_background(char* argv[])
{
    /* array of pointers to command line tokens */
    char *argvb [MAXTOKEN]; 
    pidb=fork();
    if(pidb<0)
    {
        printf("SOME ERROR HAPPENED IN FORK\n");
        exit(2);
    }
    else if(pidb==0)
    {
        sigignore(SIGINT);
        for (i = 0; i < number; i++)
        {
            if(strcmp(argv[i],"&")== 0)
            {
                argvb[i] = NULL;
            }
            else
            {
                argvb[i] = argv[i];
            }
        }
        /*redirecting stdout */
        for (i = 0; argv[i] != NULL; i++)
        {
            if(strcmp(argv[i],">")== 0)
            {
                fp = fopen(argv[i+1], "w");
                fd = fileno(fp);
                dup2(fd, STDOUT_FILENO);
            }
            if(strcmp(argv[i],"<")== 0)
            {
                fp = fopen(argv[i+1], "r");
                if (fp == NULL)
                {
                    printf("read error.\n");
                }
                fd = fileno(fp);
                dup2(fd, STDIN_FILENO);
            }
            if( (strcmp(argv[i],">")== 0) ||
                    (strcmp(argv[i],"<")== 0) )
            {
                argv[i]= NULL;
            }
        }
        condition = execvp(argvb[0],argvb);
        if(condition < 0)
        {
            switch(errno)
            {
                case ENOENT:
                    printf("COMMAND OR FILENAME NOT FOUND\n");
                    break;
                case EACCES:
                    printf("YOU DO NOT HAVE RIGHT TO ACCESS\n");
                    break;
                default:
                    printf("SOME ERROR HAPPENED IN EXEC\n");
            }
        }
        exit(3);
    }
    return 0;
}

/* implementation of commands wait I */
void waitprocess(char *argv[])
{
    if((argv[1]!=NULL) && (argv[2]==NULL))
    {
        int pid = atoi(argv[1]);
        waitpid(pid,&bgstatus,0);
    }
    else
    {
        printf("Please input the correct command\n");
    }
}
