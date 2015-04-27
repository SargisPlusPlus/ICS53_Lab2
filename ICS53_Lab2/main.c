#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/wait.h>

/* $begin main */
#define MAXARGS   128
#define	MAXLINE	 8192  /* Max text line length */

/* function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

//Main
int main()
{
    char cmdline[MAXLINE]; /* Command line */
    
    while (1) {
        /* Read */
        printf("prompt> ");
        fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);
        
        /* Evaluate */
        eval(cmdline);
    }
}
/* $end shellmain */

void unix_error(char *msg) /* Unix-style error */ {
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

pid_t Fork(void){
    pid_t pid;
    
    if ((pid = fork()) < 0)
        unix_error("Fork error");
        return pid;
}

void stdOut(char *fileName){
    FILE *fp;
    fp = fopen(fileName,"w+");
    dup2(fileno(fp),STDOUT_FILENO);
    fclose(fp);
}

void stdIn(char *fileName){
    FILE *fp;
    fp = fopen(fileName, "r");
    
    
    FILE *file;
    file = fopen("file1.txt", "w+");
    fwrite("Hi aaaaa", 10 , 1 ,file);
    fclose(file);
    
    dup2(fileno(fp),STDIN_FILENO);
    fclose(fp);
}

void stdErr(char *fileName){
    FILE *fp;
    
    fp = fopen(fileName, "w+");
    dup2(fileno(fp), STDERR_FILENO);
    fclose(fp);
}


/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline)
{
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    
    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    if (argv[0] == NULL)
        return;   /* Ignore empty lines */
    
//    FILE *fp;
//    fp = fopen("asdasd.txt", "r");
//    if (!fp){
//        printf("File not found");
//    }
//    fclose(fp);
    
    if (!builtin_command(argv)) {
        if ((pid = Fork()) == 0) {   /* Child runs user job */


            
            for (int i=0; i<MAXARGS; i++){
                if (argv[i]==NULL){
                    break;
                }
                else if (!strcmp(argv[i], ">")){
                    stdOut(argv[i+1]);
                }
                else if (!strcmp(argv[i], "<")){
                    stdIn(argv[i+1]);
                }
                else if (!strcmp(argv[i], "2>")){
                    stdErr(argv[i+1]);
                }
            }
            
            if (execv(argv[0], argv) < 0) {
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }
        
        /* Parent waits for foreground job to terminate */
        if (!bg) {
            int status;
            if (waitpid(pid, &status, 0) < 0)
                unix_error("waitfg: waitpid error");
        }
        else{
            printf("%d %s", pid, cmdline);
            // Reap bg children here using sigchild

        }
    }
    return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv)
{
    if (!strcmp(argv[0], "quit")) /* quit command */
        exit(0);
    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
        return 1;
    return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv)
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */
    
    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
        buf++;
    
    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* Ignore blank line */
        return 1;
    
    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
        argv[--argc] = NULL;

    return bg;
}
/* $end parseline */

