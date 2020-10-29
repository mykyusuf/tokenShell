#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "tokenizer.h"

pid_t childPid = 1;
int killFlag = 0;
int time = 0;
void executeShell();

void writeToStdout(char *text);

void alarmHandler(int sig);

void sigintHandler(int sig);

char *getCommandFromInput();

void registerSignalHandlers();

int killChildProcess();

void checkRedirection(char *command);

int checkPipe(char *command);

int main(int argc, char **argv)
{
    registerSignalHandlers();
    
    while (1)
    {
        executeShell();
    }

    return 0;
}

/* Sends SIGKILL signal to a child process.
 * Error checks for kill system call failure and exits program if
 * there is an error */
int killChildProcess()
{
    if (kill(childPid, SIGKILL) == -1)
    {
        return 0;
    }
    else
    {
        alarm(0);
        return 1;

    }
}

//-------------------------1B-------------------------/
/* Signal handler for SIGALRM. Catches SIGALRM signal and
 * kills the child process if it exists and is still executing.
 * It then prints out penn-shredder's catchphrase to standard output */
void alarmHandler(int sig)
{
    if (childPid != 0)
    {
        killFlag = 1;
        if (killChildProcess())
        {
            write(1, "Bwahaha ... tonight I dine on turtle soup\n", 43);
        }
    }
}

//-------------------------1B-------------------------/
/* Signal handler for SIGINT. Catches SIGINT signal (e.g. Ctrl + C) and
 * kills the child process if it exists and is executing. Does not
 * do anything to the parent process and its execution */
void sigintHandler(int sig)
{
    if (childPid != 0)
    {
        killChildProcess();
        executeShell(time);
    }
}

/* Registers SIGALRM and SIGINT handlers with corresponding functions.
 * Error checks for signal system call failure and exits program if
 * there is an error */
void registerSignalHandlers()
{
    if (signal(SIGINT, sigintHandler) == SIG_ERR)
    {
        perror("Error in signal");
        exit(EXIT_FAILURE);
    }
}

/* Prints the shell prompt and waits for input from user.
 * Takes timeout as an argument and starts an alarm of that timeout period
 * if there is a valid command. It then creates a child process which
 * executes the command with its arguments.
 *
 * The parent process waits for the child. On unsuccessful completion,
 * it exits the shell. */
void executeShell()
{
    char *command;
    int status;

    char minishell[] = "penn-shredder# ";
    writeToStdout(minishell);

    command = getCommandFromInput();
    signal(SIGALRM, alarmHandler);
    
    if (command != NULL)
    {
        childPid = fork();

        if (childPid < 0)
        {
            perror("invalid: Error in creating child process");
            exit(EXIT_FAILURE);
        }

        
        if (childPid == 0)
        {
            if (checkPipe(command)==0) {
                checkRedirection(command);
            }
            else{
                exit(EXIT_SUCCESS);
            }
                        
        }
        else
        {

            do
            {
                if (wait(&status) == -1)
                {
                    perror("invalid: Error in child process termination");
                    exit(EXIT_FAILURE);
                }
                alarm(0);

            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        
        }
    }
}

/* Writes particular text to standard output */
void writeToStdout(char *text)
{
    if (write(STDOUT_FILENO, text, strlen(text)) == -1)
    {
        perror("invalid: Error in write");
        exit(EXIT_FAILURE);
    }
}


/* Checks redirection characters in given command
 * It uses tokenizer program that given by lecturer
 * and holds the tokens in the "args" string arrays
 * If there are invalid commands like multiple inputs
 * or multiple outputs, it returns errors.
 * If there is a valid command, it uses the execvp
 * command to run given command.
 */

void checkRedirection(char *command){
    
    TOKENIZER *tokenizer;       //Tokenizer holds tokens from given command
    char *tok;                  //Tok is for get next token in tokens
    int fdOut=0,fdIn=0;         //File descriptors to get parsed input and output values from given command
    
    int checkChar=0;            //It takes the format of the command (input or output redirections)
    int largerChar=0;           // 0 if there is no > character in command and 1 if there is no > character in command
    int smallerChar=0;          // 0 if there is no < character in command and 1 if there is no < character in command
    
    tokenizer = init_tokenizer(command);    //initalize the tokenizer
    
    while( (tok = get_next_token( tokenizer )) != NULL) {

        if (!strcmp(tok,"<")) {                 //Checks how many redirection character in commands for <
            smallerChar++;
        }
        if (!strcmp(tok,">")) {                 //Checks how many redirection character in commands for >
            largerChar++;
        }
        free( tok );                            //free the token now that we're done with it
    }
    if(largerChar==1 && smallerChar==1){
      //both inut and output redirection
        checkChar=3;
    }else if(largerChar>1){
      //error if there are two output redirection
        checkChar=-1;
    }else if(smallerChar>1){
      //error if there are two input redirection
        checkChar=-2;
    }else if(largerChar==1){
      //output redirection only
        checkChar=2;
    }else if(smallerChar==1){
      //input redirection only
        checkChar=1;
    }else{
        //there is no redirection but valid command to execute
        checkChar=0;
    }
    
    if (checkChar==1) {
        
        char **args=NULL;                       //holds the command that will be called in execvp
        char **args2=NULL;                      //holds the filename that comes from input redirection
        int index=0;                            //size of args
        int index2=0;                           //size of args2
        
        tokenizer = init_tokenizer(command);                                        //initalize the tokenizer
        
        while( (tok = get_next_token( tokenizer )) != NULL && strcmp(tok,"<")) {    //checks until reaching < character
            index++;
            args = (char**)realloc(args, (index+1)*sizeof(*args));                  //reallocation args for next token
            args[index-1] = (char*)malloc(sizeof(tok));
            strcpy(args[index-1], tok);
            free( tok );                                                            //free the token now that we're done with it
        }
        
        while( (tok = get_next_token( tokenizer )) != NULL ) {                      //checks until reaching NULL
            index2++;
            args2 = (char**)realloc(args2, (index2+1)*sizeof(*args2));              //reallocation args2 for next token
            args2[index2-1] = (char*)malloc(sizeof(tok));
            strcpy(args2[index2-1], tok);
            free( tok );                                                            //free the token now that we're done with it
        }
                
        if((fdIn = open(args2[0], O_RDONLY , 0644)) < 0){                           //opens input file and assigns file descriptor
            perror("invalid standard input redirect");
            exit(EXIT_FAILURE);
        }
        
        dup2(fdIn, 0);                                                              //fdIn is set for standard input
        close(fdIn);
         
        if (execvp(args[0],&args[0])<0) {                                           //Execute the command with our new input fdIn
            perror("invalid: Wrong arguments for execvp");
        }
        
        free_tokenizer( tokenizer );                                                //free the token now that we're done with it
        free(args);                                                                 //free the args now that we're done with it
        free(args2);                                                                //free the args2 now that we're done with it
        exit(1);

    }
    
    else if (checkChar==2) {
        
        char **args=NULL;                       //holds the command that will be called in execvp
        char **args2=NULL;                      //holds the filename that comes from input redirection
        int index=0;                            //size of args
        int index2=0;                           //size of args2
        
        tokenizer = init_tokenizer(command);                                        //initalize the tokenizer
        
        while( (tok = get_next_token( tokenizer )) != NULL && strcmp(tok,">")) {    //checks until reaching > character
            index++;
            args = (char**)realloc(args, (index+1)*sizeof(*args));                  //reallocation args for next token
            args[index-1] = (char*)malloc(sizeof(tok));
            strcpy(args[index-1], tok);
            free( tok );                                                            //free the token now that we're done with it
        }
        
        while( (tok = get_next_token( tokenizer )) != NULL ) {                      //checks until reaching NULL
            index2++;
            args2 = (char**)realloc(args2, (index2+1)*sizeof(*args2));              //reallocation args2 for next token
            args2[index2-1] = (char*)malloc(sizeof(tok));
            strcpy(args2[index2-1], tok);
            free( tok );                                                            //free the token now that we're done with it
        }
                
        if((fdOut = open(args2[0], O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0){       //opens output file and assigns file descriptor
            perror("invalid standard output redirect");                             //it creates if there is no given file, truncates
                                                                                    //file content and allowed writeng only
            exit(EXIT_FAILURE);
        }
        
        dup2(fdOut, 1);                                                             //fdOut is set for standard output
        close(fdOut);
        
        if (execvp(args[0],&args[0])<0) {                                           //Execute the command with our new output fdOut
            perror("invalid: Wrong arguments for execvp");
        }
        
        free_tokenizer( tokenizer );                                                //free the token now that we're done with it
        free(args);                                                                 //free the args now that we're done with it
        free(args2);                                                                //free the args2 now that we're done with it
        exit(1);

    }
    else if (checkChar==3) {
        
        char **args=NULL;                       //holds the command that will be called in execvp
        char **args2=NULL;                      //holds the filename that comes from input or output redirection
        char **args3=NULL;                      //holds the filename that comes from input or output redirection
        
        char holder[2];                         //checks which "args" is for input and which "args" is for output
        
        int index=0;                            //size of args
        int index2=0;                           //size of args2
        int index3=0;                           //size of args3
        
        tokenizer = init_tokenizer(command);                                        //initalize the tokenizer
        
        while( (tok = get_next_token( tokenizer )) != NULL && (  strcmp(tok,"<") && strcmp(tok,">") ) ) {
                                                                                    //checks until reaching > or < character
            index++;
            args = (char**)realloc(args, (index+1)*sizeof(*args));                  //reallocation args for next token
            args[index-1] = (char*)malloc(sizeof(tok));
            strcpy(args[index-1], tok);
                    
        }
        
        if (!strcmp(tok,"<")) {                                                     //checks which character is found
            holder[0]='<';
        }
        else{
            holder[0]='>';
        }
        free( tok );                                                                //free the token now that we're done with it

        
        while( (tok = get_next_token( tokenizer )) != NULL && ( strcmp(tok,">") && strcmp(tok,"<") ) ) {
                                                                                    //checks until reaching > or < character
            index2++;
            args2 = (char**)realloc(args2, (index2+1)*sizeof(*args2));              //reallocation args2 for next token
            args2[index2-1] = (char*)malloc(sizeof(tok));
            strcpy(args2[index2-1], tok);
        }
        
        if (!strcmp(tok,"<")) {                                                     //checks which character is found
            holder[1]='<';
        }
        else{
            holder[1]='>';
        }
        
        free( tok );                                                                //free the token now that we're done with it

        while( (tok = get_next_token( tokenizer )) != NULL ) {                      //checks until reaching NULL
            index3++;
            args3 = (char**)realloc(args3, (index3+1)*sizeof(*args3));              //reallocation args3 for next token
            args3[index3-1] = (char*)malloc(sizeof(tok));
            strcpy(args3[index3-1], tok);
              
        }
                        
        free( tok );                                                                //free the token now that we're done with it
                
        if (holder[0]=='<' && holder[1]=='>') {                                     //checks the sides of the redirections
                
            if((fdIn = open(args2[0], O_RDONLY , 0644)) < 0){                       //opens input file and assigns file descriptor
                perror("invalid standard input redirect");
                exit(EXIT_FAILURE);
            }
            
            if((fdOut = open(args3[0], O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0){   //opens output file and assigns file descriptor
                perror("invalid standard output redirect");                         //it creates if there is no given file, truncates
                exit(EXIT_FAILURE);                                                 //file content and allowed writeng only
            }
            
            dup2(fdOut, 1);                                                         //fdOut is set for standard output
            close(fdOut);
            
            dup2(fdIn, 0);                                                          //fdIn is set for standard input
            close(fdIn);
                
            if (execvp(args[0],&args[0])<0) {                                       //Execute the command with our new output fdOut
                perror("invalid: Wrong arguments for execvp");                      // and our new input fdIn
            }
            
            free_tokenizer( tokenizer );                                            //free the token now that we're done with it
            free(args);                                                             //free the args now that we're done with it
            free(args2);                                                            //free the args2 now that we're done with it
            free(args3);                                                            //free the args3 now that we're done with it
            exit(1);

            
        }
        else if (holder[1]=='<' && holder[0]=='>') {                                //checks the sides of the redirections
            
            if((fdIn = open(args3[0], O_RDONLY , 0644)) < 0){                       //opens input file and assigns file descriptor
                perror("invalid standard input redirect");
                exit(EXIT_FAILURE);
            }
            
            if((fdOut = open(args2[0], O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0){   //opens output file and assigns file descriptor
                perror("invalid standard output redirect");                         //it creates if there is no given file, truncates
                exit(EXIT_FAILURE);                                                 //file content and allowed writeng only
            }
            
            dup2(fdOut, 1);                                                         //fdOut is set for standard output
            close(fdOut);
            
            dup2(fdIn, 0);                                                          //fdIn is set for standard input
            close(fdIn);
            
            if (execvp(args[0],&args[0])<0) {                                       //Execute the command with our new output fdOut
                perror("invalid: Wrong arguments for execvp");                      // and our new input fdIn
            }
            
            free_tokenizer( tokenizer );                                            //free the token now that we're done with it
            free(args);                                                             //free the args now that we're done with it
            free(args2);                                                            //free the args2 now that we're done with it
            free(args3);                                                            //free the args3 now that we're done with it
            exit(1);

        }
        else{                                                                       //There is a multiple redirection error
            
            perror("invalid: Multiple standard redirects or redirect in invalid location");
            free_tokenizer( tokenizer );                                            //free the token now that we're done with it
            free(args);                                                             //free the args now that we're done with it
            free(args2);                                                            //free the args2 now that we're done with it
            free(args3);                                                            //free the args3 now that we're done with it
            exit(EXIT_FAILURE);
        }
                        
        exit(1);

    }
    
    else if(checkChar==0){                                                          //Execute command without redirection
        
        char **args=NULL;                                                           //holds the command that will be called in execvp
        int index=0;                                                                //size of args
        
        tokenizer = init_tokenizer(command);                                        //initalize the tokenizer
        
        while( (tok = get_next_token( tokenizer )) != NULL ) {                      //checks until reaching NULL
            index++;
            args = (char**)realloc(args, (index+1)*sizeof(*args));                  //reallocation args3 for next token
            args[index-1] = (char*)malloc(sizeof(tok));
            strcpy(args[index-1], tok);
        }
        free( tok );                                                                //free the args now that we're done with it
        
        if (execvp(args[0],&args[0])<0) {                                           //Execute the command
            perror("invalid: Wrong arguments for execvp");
        }
        
        free_tokenizer( tokenizer );                                                //free the token now that we're done with it
        free(args);                                                                 //free the args now that we're done with it
        exit(1);
    }
    else if(checkChar==-1){                                                         //Multiple output error
        perror("invalid: Multiple standard output redirects");
        exit(EXIT_FAILURE);
    }
    else{                                                                           //Multiple input error
        perror("invalid: Multiple standard input redirects or redirect in invalid location");
        exit(EXIT_FAILURE);
    }
    exit(0);

}

int checkPipe(char *command){

    int fd[2];                  //file descriptors
    pid_t  leftChild,rightChild;//Child processes
    
    TOKENIZER *tokenizer;       //Tokenizer holds tokens from given command
    
    char *tok;
    char *cmd1;                 //Left side of the pipeline
    char *cmd2;                 //Right side of the pipeline
    
    int rstatus;                //status of the first child

    int largerChar=0;           // 0 if there is no > character in command and 1 if there is > character in command
    int smallerChar=0;          // 0 if there is no < character in command and 1 if there is < character in command
    int pipeChar=0;             // 0 if there is no | character in command and 1 if there is | character in command
    
    tokenizer = init_tokenizer(command);    //initalize the tokenizer
    
    while( (tok = get_next_token( tokenizer )) != NULL) {

        if (!strcmp(tok,"<")) {                 //Checks how many redirection character in commands for <
            smallerChar++;
        }
        if (!strcmp(tok,">")) {                 //Checks how many redirection character in commands for >
            largerChar++;
        }
        if (!strcmp(tok,"|")) {                 //Checks how many pipe character in commands for |
            pipeChar++;
        }
        free( tok );                            //free the token now that we're done with it
    }

    if (smallerChar<2 && largerChar<2 && pipeChar<2) {      //if there are invalid commands it returns error

        if (strstr(command,"|")!=NULL) {        //checks for pipe command
            
            cmd1=strtok(command,"|");           //splits command for left side and right side
            cmd2=strtok(NULL,"");
            
            
            if(strstr(&cmd2[1],"<")!=NULL){     //checks if there is invalide pipeline or double input redirection
                
                if (strstr(cmd1,">")!=NULL) {
                    perror("many invalid pieline");
                    exit(EXIT_FAILURE);
                }
                else{
                    perror("invalid standard input redirect");
                    exit(EXIT_FAILURE);
                }
            }
            if(strstr(cmd1,">")!=NULL){         //checks if there is invalide pipeline or double output redirection
                
                if (strstr(&cmd2[1],"<")!=NULL) {
                    perror("many invalid pieline");
                    exit(EXIT_FAILURE);
                }
                else{
                    perror("invalid standard output redirect");
                    exit(EXIT_FAILURE);
                }
            }
            
            if((pipe(fd)<0)){                   //if there is no error, it creates a pipe
                perror("Error creating pipe.\n");
                exit(EXIT_FAILURE);
            }

            if((leftChild=fork())<0){           //forks to run commands splitted before
                perror("Error forking.\n");
                exit(EXIT_FAILURE);
            }
            
            //in child process
            if(leftChild==0){

                dup2(fd[1], 1);                 //Closes other side of pipe and waits for writing
                close(fd[0]);
                close(fd[1]);
                
                checkRedirection(cmd1);         //Child process executes left side of the pipe
                
                
            }
            if(leftChild>0){
                
                int cstatus;
                
                dup2(fd[0], 0);                 //Closes other side of pipe and waits for reading
                close(fd[1]);
                close(fd[0]);
                
                rightChild = fork();                     //Parent process forks again to execute right side of the pipe
                
                if (rightChild < 0)                      //checks for fork creation
                {
                    perror("invalid: Error in creating child process");
                    exit(EXIT_FAILURE);
                }
                
                if (rightChild == 0)
                {
                    checkRedirection(&cmd2[1]); //Child process executes right side of the pipe
                }
                else
                {
                    do
                    {
                        if (wait(&cstatus) == -1)       //parent process waits for child process coming from right side of the pipe
                        {
                            perror("invalid: Error in child process termination");
                            exit(EXIT_FAILURE);
                        }
                        alarm(0);

                    } while (!WIFEXITED(cstatus) && !WIFSIGNALED(cstatus)); //checks status of the child process
                    
                    do
                    {
                        if (wait(&rstatus) == -1)       //parent process waits for child process coming from left side of the pipe
                        {
                            perror("invalid: Error in child process termination");
                            exit(EXIT_FAILURE);
                        }
                        alarm(0);

                    } while (!WIFEXITED(rstatus) && !WIFSIGNALED(rstatus)); //checks status of the child process
                    
                }
                                
            }
            
            return 1;                                   //if there is a valid pipe, it returns 1
        }
        else{
            return 0;                                   //if there is a no pipe, it returns 0 and runs command in other function
        }
    }
    
    else if(pipeChar>=2){                               //if there are multiple pipe commands, returns error
        perror("invalid pipeline");
        exit(EXIT_FAILURE);
    }
    else if(smallerChar>=2){                            //if there are multiple input redirection commands, returns error
        perror("invalid input redirection");
        exit(EXIT_FAILURE);

    }
    else if(largerChar>=2){                             //if there are multiple output redirection commands, returns error
        perror("invalid output redirection");
        exit(EXIT_FAILURE);
    }
        
    perror("invalid pipeline");                         //invalid pipeline
    exit(EXIT_FAILURE);
    
    return 1;
}




/* Reads input from standard input till it reaches a new line character.
 * Checks if EOF (Ctrl + D) is being read and exits penn-shredder if that is the case
 * Otherwise, it checks for a valid input and adds the characters to an input buffer.
 *
 * From this input buffer, the first 1023 characters (if more than 1023) or the whole
 * buffer are assigned to command and returned. An \0 is appended to the command so
 * that it is null terminated */
//-------------------------1A-------------------------/
char *getCommandFromInput()
{
    char *buff = malloc(1024 * sizeof(char));

    int temp = read(0, buff, 1024);
    if (temp == 0)
    {
        printf("^D\n");
        exit(0);
    }
    else
    {
        buff[temp - 1] = '\0';
    }

    return buff;
}
