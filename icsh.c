// #Pariwat Huang 6180067
// Reference : 
//          https://github.com/szholdiyarov/command-line-interpreter/blob/master/myshell.c
//          https://stackoverflow.com/questions/58361506/save-history-command-on-simple-shell-by-c-code
//          https://stackoverflow.com/questions/52939356/redirecting-i-o-in-a-custom-shell-program-written-in-c
//          http://people.cs.pitt.edu/~khalifa/cs449/spr07/Assigns/Assign4/myshell.c
// 			https://github.com/hungys/mysh/blob/master/mysh.c
// 			https://www.gnu.org/software/libc/manual/html_node/Initializing-the-Shell.html
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>   // Definition for fork() and execve()
#include <errno.h>    // Definition for "error handling"
#include <sys/wait.h> // Definition for wait()
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>


#define MAXCHAR 1000
#define NR_JOBS 20
#define BACKGROUND_EXECUTION 0
#define FOREGROUND_EXECUTION 1

struct process {
    char *command;
    int argc;
    char **argv;
    char *input_path;
    char *output_path;
    pid_t pid;
    int type;
    int status;
    struct process *next;
};

struct job {
    int id;
    struct process *root;
    char *command;
    pid_t pgid;
    int mode;
};


    /* Declarations for getline() */
    char *input = NULL;
    size_t capline = 0; // Capacity

    /* Declaration for strtok() */
    int i;
    int j;
    int err;

    // For condition in checkRedirect
    int cond;

    //For Ampersand Condition
    int block;
    
    //Declaration for file
    FILE *fp;
    char *cp;
    char *ifile;
    char *ofile;
    char *args2[512];

    // count numbers of indices in prev array
    int count;
    int prev_count;
    char *token;
    char *array[512]; 
    // For previous command
    char *prev[512]; 



    // void checkForFile(char const *argv[])
    // {
    //  char str[MAXCHAR];

    //  fp = fopen(argv[1], "r");
    //  // if (fp == NULL){
    //  //     printf("Could not open file %s",argv[1]);
    //  // }
    //  while (fgets(str, MAXCHAR, fp) != NULL)
    //      printf("%s", str);
    //  fclose(fp);
    // }

   void printArr(char** array){
        for(int i = 0; i < sizeof(array); i++){
            printf("Array %d %s\n",i,array[i]);
        }
   }

    /* Display some info to the user at the start */
    void startDisplay(){
    printf("\n");
    printf("Starting IC Shell                                            \n");
    printf("\n");
    }

    /* Print out "MY_SHELL" */
    void displayPrompt(){
        printf("icsh $ ");
    }

    /* Divide input line into tokens */
    void makeTokens(char *input){
        i = 0;
        // idx = 0
        token = strtok(input, "\n ");
            while (token != NULL) { 
            if(strcmp(token,"!!") == 0) {

            }else{
                count++;
            }               
            array[i++] = token; // Add tokens into the array
            token = strtok(NULL, "\n ");
            }
        array[i] = NULL;
    }

    /* Execute a command */
   void execute(){
   		int status;
        int pid = fork(); // Create a new process
        if (pid != 0) { // If not successfully completed
                int s;
                waitpid(-1, &s, 0); // Wait for process termination
        } else {
            if (ifile != NULL) {
                int fd = open(ifile, O_RDONLY);

                if (dup2(fd, STDIN_FILENO) == -1) {
                    fprintf(stderr, "dup2 failed");
                }

                close(fd);
            }

            // trying to get this to work
            // NOTE: now it works :-)
            // open stdout
            if (ofile != NULL) {
                // args[1] = NULL;
                int fd2;

                //printf("PLEASE WORK");
                if ((fd2 = open(ofile, O_WRONLY | O_CREAT, 0644)) < 0) {
                    perror("couldn't open output file.");
                    exit(0);
                }
                dup2(fd2, STDOUT_FILENO);
                close(fd2);
            }


            if(execvp(array[0], array) == -1){ // If returned -1 => something went wrong! If not then command successfully completed */
                perror("Wrong command"); // Display error message
                exit(errno);
            }               
        }
        // Wait for the child process to complete, if necessary
          if(block) {
            printf("Waiting for child, pid = %d\n", pid);
            waitpid(pid, &status, 0);
          }

   }

   void copyArr(char** des, char** src){
        //Reset the firs array
        for(int i = 0; des[i] != NULL; i++){
            des[i] = NULL;
        }
        for(int j = 0; src[j] != NULL; j++){
            des[j] =  strdup(src[j]);
        }

   }


   void checkBangs(char** argv , char** history)
   {
    if(strcmp(argv[0], "!!") == 0){
        if(history[0] == NULL)
        {
            printf("Test");
        }else
        {
            // printf("Argv\n");
            // printArr(argv);
            // printf("history\n");
            // printArr(history);
            copyArr(argv,history);
            // printf("After\n");
            // printf("\n");
            // printf("Argv\n");
            // printArr(argv);

        }
    }
    else{
        prev_count = count;
        // If the length is one then do something else 
        copyArr(history,argv);
        count = 0;
    }
 
   }
   void sigintHandler(int sig_num)
   {
        write(1, "\n", 1);
   }

   void sigstopHandler(int sig_num){
        printf("\n");
   }

   void sigChildHandler(int sig_num)
   {
   	int status;
   	int result = wait(&status);

   	printf("Wait returned %d\n", result);
   }

   void checkRedirect(char** args, char** args2){
      
        // printf("Args\n");
        // printArr(args);
        // assume no redirections
        ofile = NULL;
        ifile = NULL;

        // split off the redirections
        j = 0;
        i = 0;
        cond = 0;
        err = 0;
        while (1) {
            cp = args[i++];
            // printf("Initial cp:  %s \n",cp );
            if (cp == NULL)
                break;

            switch (*cp) {
            case '<':
                if (cp[1] == 0)
                    cp = args[i++];
                else
                    ++cp;
                    cond = 1;
                ifile = cp;
                if (cp == NULL)
                    err = 1;
                else
                    if (cp[0] == 0)
                        err = 1;
                break;

            case '>':

                if (cp[1] == 0)
                    cp = args[i++];
                else
                    ++cp;
                    cond = 1;
                ofile = cp;
                if (cp == NULL)
                    err = 1;
                else
                    if (cp[0] == 0)
                        err = 1;
                break;

            default:
                // printf(" Args2[j++] :  %s \n",cp );
  
                args2[j++] = cp;
                break;
            }
        }

        args2[j] = NULL;
        // printf("Args2\n");
        // printArr(args2);
        // printf("Condition : %d \n",cond );
        if(cond == 1){
            // printf("Yes\n");
            copyArr(array,args2);
        }
        // printf("Array\n");
        // printArr(array);

   }

   int checkAmpersand(char** args){
   	for( i = 0; args[i] != NULL; i++){
   		if(args[i][0] == '&'){
   			free(args[i]);
   			args[i] = NULL;
   			return 1;
   		}else{
   			return 0;
   		}
   	}
   	return 0;
   }





    int main(int argc, char const *argv[]){

        if(argv[1] != NULL){
            char str[MAXCHAR];

            fp = fopen(argv[1], "r");
            // if (fp == NULL){
            //     printf("Could not open file %s",argv[1]);
            // }
            while (fgets(str, MAXCHAR, fp) != NULL){
                // printf("Print: %s \n", str);
                makeTokens(str);
                if (strcmp(array[0], "exit") == 0) {
                    return atoi(array[1]);
                }
                checkBangs(array,prev);

                execute();
                
            }
            fclose(fp);     
        }

        else{
            sigaction(SIGINT, &(struct sigaction){ .sa_handler = sigintHandler }, NULL);
            sigaction(SIGTSTP, &(struct sigaction){.sa_handler = sigstopHandler}, NULL);
            sigaction(SIGCHLD,  &(struct sigaction){.sa_handler = sigChildHandler}, NULL);
            startDisplay();

            while(1){
                displayPrompt(); // Display a user prompt
                getline(&input, &capline, stdin); // Read the user input
                /* Check if input is empty */
                if(strcmp(input,"\n")==0){
                    continue;
                }

                makeTokens(input); // Divide line into tokens 

                /* Check if input is "q", if yes then exit shell */
                if (strcmp(array[0], "exit") == 0) {
                    return atoi(array[1]);
                }

                block = (checkAmpersand(array) == 0);
                //Checking for redirection
                ofile = NULL;
                ifile = NULL;
                checkRedirect(array,args2);
                // printf("Before\n");
                // printArr(array);
                // printf("After\n");
                // printArr(array);
                checkBangs(array,prev);


                // printf("Prev\n");
                // printArr(prev);

                execute(); // Call execvp()

            }
        // }
        }
    }


// } 