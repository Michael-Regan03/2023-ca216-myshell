//NAME: Michael Regan
//Student ID: 22112111
//I Michael Regan acknowledge of the DCU Academic Integrity Policy

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include<fcntl.h>



#define MAX_BUFFER 1024                        // max line buffer [2]
#define MAX_ARGS 64                            // max # args [2]
#define SEPARATORS " \t\n"                     // token separators [2]


FILE *freopen(const char * pathname, const char * type, FILE *fp);  //declaring freopen [2]


/* Internal Commands*/
void clr(pid_t pid);  
void dir(pid_t pid,char** args);
void cd(pid_t pid, char ** args, char *prompt);
void echo(pid_t pid, char ** args);
void shellPause(pid_t pid, char ** args);
void environment(pid_t pid, char ** args);

int STDOUT( char **args);
int STDIN( char **args);

void executables(pid_t pid, char ** args);

int main (int argc, char ** argv)
{
    char buf[MAX_BUFFER];                      // line buffer
    char * args[MAX_ARGS];                     // pointers to arg strings
    char ** arg = malloc(MAX_ARGS * sizeof(char*)); // working pointer thru args
    char prompt[MAX_BUFFER];                        //Terminal prompt
    getcwd(prompt,MAX_BUFFER);                    //using are current working directory as the shell prompt
    char* p = "$ "; //the end of the shell prompt
    
    /* keep reading input until "quit" command or eof of redirected input */
    while (!feof(stdin)) { 
        fputs (prompt , stdout); // write prompt
        printf("%s", p);
        if (fgets (buf, MAX_BUFFER, stdin )) { // read a line
            /* tokenize the input into args array */
            arg = args;
            *arg++ = strtok(buf,SEPARATORS);   // tokenise input

            while ((*arg++ = strtok(NULL,SEPARATORS)));
            
            pid_t pid = fork(); //creating child process

            // last entry will be NULL 
            if (args[0]) {            // if there's anything arguments
                /* check for internal/external command */

                if (!strcmp(args[0],"clr")) {  // "clr" command
                    clr(pid);
                    continue;
                }else if(strcmp(args[0], "dir") == 0) { // "dir" command
                    dir(pid, args);
                    continue;
                }else if(strcmp(args[0], "cd")==0) {     // "cd" command
                    cd(pid, args,prompt);
                    continue;
                }else if(strcmp(args[0], "echo")==0) {     // "echo" command
                    echo(pid, args);
                    continue;
                }else if(strcmp(args[0], "pause")==0) {     // "pause" command
                    shellPause(pid, args);
                    continue;
                }else if(!strcmp(args[0],"quit")){  // "quit" command
                    break;                     // break out of 'while' loop
                }else if(strcmp(args[0], "environ") == 0) {
                    environment(pid, args);
                }else{
                    executables(pid,args); //execute external command
                    continue;
                }
                
                if(pid==0){
                    exit(0); //closing child process
                }
                arg = args;
            }
        }
    }
    return 0; 
}


void clr(pid_t pid){
if (pid == 0) { //if its the child process
    char *cmd[] = {"clear", NULL};  //storing data for execvp() in an array [3]
    execvp(cmd[0],cmd); //executing clear 
    exit(0); //exiting child process
} else {
    waitpid(pid, NULL, 0);  //parent process waits for child process[5] 
    }
}

void dir(pid_t pid, char **args) {
    if (pid == 0) { //if its the child process
        int x = STDOUT(args); //checking if a file is included for standard output
        char *cmd[4]; ///array for execvp[4]
        cmd[0] = "ls"; //command ls
        cmd[1] = "-al"; //first argument -al
        if (args[1] != NULL){  //if there is more then one argument
            if(strcmp(args[1], ">") != 0 && strcmp(args[1], ">>") != 0){ //if the second argument isn't a file to write too
                cmd[2]= args[1]; //second argument is a PATH
                cmd[3] = NULL; //last argument is NULL
            
            }else{
                cmd[2] = NULL; //second argument is null even tho it might exist as a file to write to however standard output has already been changed in STDOUT()
            }
        }else{
            cmd[2] = NULL; //second argument is NULL
        }
        execvp(cmd[0], cmd); //executing ls with arguments [4]
        exit(0); //exiting child process
    } else {
        waitpid(pid, NULL, 0);  //parent process waits for child process[5]
    }
}

void cd(pid_t pid, char ** args, char *prompt){
    if(pid == 0){
        if (args[1] != NULL) {        // if "cd" command has an argument
        if(strcmp(args[1], "-")==0){
            chdir("..");
        }else{
            chdir(args[1]);           // change the directory to the specified argument
        }
            getcwd(prompt, MAX_BUFFER);  // update the prompt to the current working directory
            setenv("PWD", prompt, 1);     // set the PWD environment variable
        }
        exit(0); //exiting child process
    }else {
        waitpid(pid, NULL, 0);  //parent process waits for child process[5]
    }
}

void echo(pid_t pid, char ** args){
    if(pid == 0){
    int x = STDOUT(args);
    for(int i=0; args[1+i] != NULL;++i ){
        printf("%s ",args[1+i]);
    }
    printf("\n");
    exit(0); //exiting child process
    }else {
        waitpid(pid, NULL, 0); //parent process waits for child process[5]
    }
}

void shellPause(pid_t pid, char ** args){
    if(pid == 0){
        while (getchar() != '\n'); //loop until ENTER is pressed
        exit(0); //exit child process
    }else {
        waitpid(pid, NULL, 0); //parent process waits for child process[5]
    }
}



void  environment(pid_t pid, char ** args){
    if(pid == 0){ //if were in the child process
        int x = STDOUT(args); //checking to see if output file is an argument
        extern char **environ; //global variable [2]
        int i = 0; 
        while (environ[i]) { //looping through environment variables
            printf("%s\n", environ[i++]); //printing environment variables
        }
    }else {
        waitpid(pid, NULL, 0); //parent process waits for child process[5]
    }
}

int STDOUT( char **args){
    for(int i = 0; args[i]!=NULL; ++i){ //for all the arguments passed
        if(strcmp(args[i], ">") == 0){ 
            FILE *file = freopen(args[i+1], "w", stdout); //setting our standard output to the file[2]
            if (file == NULL) {
                    perror("freopen"); //error if opening the file fails 
                    exit(1); //closing process
                }
            return 1; //return one to signal standard output has been changed
        }else if(strcmp(args[i], ">>") == 0){
            FILE *file = freopen(args[i+1], "a", stdout); //setting our standard output to append to the file[2]
            if (file == NULL) {
                perror("freopen");  //error if opening the file fails 
                exit(1); //closing process
                }
    return 1; //return one to signal standard output has been changed
    }  
    }
    return 0;
}

int STDIN( char **args){
    for(int i = 0; args[i]!=NULL; ++i){ //for all the arguments passed
        if(strcmp(args[i], "<") == 0){
                FILE *file = freopen(args[i+1], "r", stdin); //setting our standard input to the file[2]
                if (file == NULL) {
                    perror("freopen"); //error if opening the file fails 
                    exit(1); //closing process
                }
                return 1; //return one to signal standard input has been changed
            }else if(strcmp(args[i], "<<") == 0){
                FILE *file = freopen(args[i+1], "r", stdin); //setting our standard input to the file[2]
                if (file == NULL) {
                    perror("freopen"); //error if opening the file fails 
                    exit(1); //closing process
                }
                return 1; //return one to signal standard input has been changed
            }
        }
    return 0;
}

void executables(pid_t pid, char ** args){
    if(pid == 0){
        int count  = 0;
        
        int flag1 = STDIN(args); //checking for files to read
        int flag2 =STDOUT(args); //checking for files to write too

        for( int i =0; args[i]!=NULL; ++i){ //counting the amount of arguments passed to the executable
            ++count;
        }

        if(flag1==1){
            count = count-2; //removing input file from arguments
        }
        if(flag2==1){
            count = count-2; //removing output file from arguments
        }

        char* arr[count+1]; //initialising array of arguments for execvp()
           
            for(int i=0; i < count ; ++i){
                arr[i] = args[i];
            }
            arr[count+1] = NULL; //last item in the list has to be NULL[4]
 

        if(execvp(arr[0], arr) == -1){ //executes command and error tests
            perror("execvp"); //error message
            exit(1); //exit child process
        }
        exit(0); //exit child process
    }else {
        waitpid(pid, NULL, 0); //parent process waits for child process[5]
    }
}

