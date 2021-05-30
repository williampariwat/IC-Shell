// #Pariwat Huang 6180067
// Reference : 
         // https://github.com/szholdiyarov/command-line-interpreter/blob/master/myshell.c
         // https://stackoverflow.com/questions/58361506/save-history-command-on-simple-shell-by-c-code
         // https://stackoverflow.com/questions/52939356/redirecting-i-o-in-a-custom-shell-program-written-in-c
         // http://people.cs.pitt.edu/~khalifa/cs449/spr07/Assigns/Assign4/myshell.c
         // https://github.com/hungys/mysh/blob/master/mysh.c
         // https://www.gnu.org/software/libc/manual/html_node/Initializing-the-Shell.html

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


#define STATUS_RUNNING 0
#define STATUS_DONE 1
#define STATUS_SUSPENDED 2
#define STATUS_CONTINUED 3



#define MAXCHAR 1000
#define NR_JOBS 20

struct job
{
    char name[100];
    int pid;
    int status;
    int is_back;
};


/*---------------
  Setting up job
-----------------*/
struct job back[100];
struct job fore;
int back_count = 0;
int child_count = 0;

int shellid = 0;
int mode;
int switcher = 0;
/*--------------------
   Setting up childpid
---------------------*/
int childpid = -1;


/*---------------
Getline Declaration
-----------------*/
char *input = NULL;
size_t capline = 0; 

int i;
int j;
int err;


/*----------------
  For Checkredirect
-----------------*/
int cond;
int set_count;
/*------------------------
   For ampersand condition
---------------------------*/

int block;


/*----------------------
   Declaration for file
------------------------*/
FILE *fp;
char *cp;


/*-------------------
  Check I/O
-----------------*/
char *ifile;
char *ofile;



char *args2[512];



char *token;
char *array[512]; 

/*-------------------
  For previous command
-----------------*/

char *prev[512]; 



/*------------------------------
Copy from des array to src array
-------------------------------*/
void copyArr(char** des, char** src){
        //Reset the firs array

        for(int i = 0; des[i] != NULL; i++){
            des[i] = NULL;
        }



        for(int j = 0; src[j] != NULL; j++){
            des[j] =  strdup(src[j]);
        }



   }

/*------------------------
   Print Array for testing
--------------------------*/
void printArr(char** array){
        for(int i = 0; i < sizeof(array); i++){
            printf("Array %d %s\n",i,array[i]);
        }
   }

     // Display some info to the user at the start 
    void startDisplay(){
    printf("\n");
    printf("Starting IC Shell                                            \n");
    printf("\n");
    }

/*---------------
   Display Prompt
-----------------*/
void displayPrompt(){
    printf("icsh $ ");
}
/*---------------------------
Divide input line into tokens
----------------------------*/

void makeTokens(char *input){
        i = 0;
        // idx = 0
        token = strtok(input, "\n ");
            while (token != NULL) { 
            // if(strcmp(token,"!!") == 0) {

            // }else{
            //     count++;
            // }               
            array[i++] = token; // Add tokens into the array
            token = strtok(NULL, "\n ");
            }
        array[i] = NULL;
    }
/*-------------
    Exceutor
---------------*/
void execute(int k){
        int status;
        int pid = fork(); // Create a new process
        if (pid < 0) { // If not successfully completed
            printf("Error: Fork Failed\n");
            exit(0);
        } else if(pid == 0) {
            setpgid(0, 0);

            if (ifile != NULL) {

                array[0] = args2[0];
                if(set_count < 4){
                    for(i = 1; array[i] != NULL; i++){
                        array[i] = NULL;
                    }
                }else{
                    for(i = 2; array[i] != NULL; i++){
                        array[i] = NULL;
                    }
                }

                int fd = open(ifile, O_RDONLY);

                if (dup2(fd, STDIN_FILENO) == -1) {
                    fprintf(stderr, "dup2 failed");
                }

                close(fd);
            }

            if (ofile != NULL) {

                array[0] = args2[0];
                if(set_count < 4){
                    for(i = 1; array[i] != NULL; i++){
                        array[i] = NULL;
                    }
                }else{
                    for(i = 2; array[i] != NULL; i++){
                        array[i] = NULL;
                    }
                }

                int fd2;

                if ((fd2 = open(ofile, O_WRONLY | O_CREAT, 0644)) < 0) {
                    perror("couldn't open output file.");
                    exit(0);
                }

                dup2(fd2, STDOUT_FILENO);

                close(fd2);
                printf("Done \n");
            }


            if(execvp(array[0], array) == -1){ // If returned -1 => something went wrong! If not then command successfully completed 
                perror("Wrong command"); // Display error message
                exit(errno);
            }               
        }
        else{
            int x;
            childpid = pid;
            char name[100];
            strcpy(name, array[0]);

            for (i = 1; i < k; i++)
            {
                strcat(name, " ");
                strcat(name, array[i]);
            }

            fore.pid = pid;
            strcpy(fore.name, name);
            fore.is_back = 0;
            waitpid(-1, NULL, WUNTRACED);
        }

   }


/*-----------------------------
Check for bang and then execute
-------------------------------*/
void checkBangs(char** argv , char** history)
   {
    if(strcmp(argv[0], "!!") == 0){
        if(history[0] == NULL)
        {
            printf("Test");
        }else
        {        
            copyArr(argv,history);
        }
    }
    else{
        copyArr(history,argv);
    }

   }

/*-------------
Signal Handler
---------------*/

void ctrl_z(int signo)
    {   
        printf("\n");
        pid_t p = getpid();
        if (p != shellid)
            return;
        //print();
        if (childpid != -1)
        {
            kill(childpid, SIGTTIN);
            kill(childpid, SIGTSTP);
            back_count++;
            back[back_count].pid = childpid;
            back[back_count].is_back = 1;
            strcpy(back[back_count].name, fore.name);
            back[back_count].status = STATUS_SUSPENDED;
            printf("[%d]+ Stopped                 %s \n",back_count, back[back_count].name );
        }

        signal(SIGTSTP, ctrl_z);
}

void sigintHandler(int sig_num)
   {
        write(1, "\n", 1);
   }




void child_sig(int signo)
{
    pid_t pid;
    int x;
    pid = waitpid(WAIT_ANY, &x, WNOHANG);
    int i;
    for (i = 1; i <= back_count; i++)
    {
        if (back[i].pid == pid)
        {
            int exit_status = WEXITSTATUS(x);
            if (exit_status == 0){
                child_count++;
                back[i].status = STATUS_DONE;
                printf("\n[%d]+ Done                    %s \n", child_count,back[i].name);
                printf("icsh $ "); 
            }
            else{
                printf("%s with pid %d exited with exit status %d\n", back[i].name, back[i].pid, exit_status);
            }
            fflush(stdout);
            break;
        }

    }
    signal(SIGCHLD, child_sig);
}



/*------------------
Redirection
-------------------*/

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
                printf("ofile : %s \n",ofile);
                if (cp == NULL)
                    err = 1;
                else
                    if (cp[0] == 0)
                        err = 1;
                break;

            default:  
                args2[j++] = cp;
                break;
            }
        }

        args2[j] = NULL;

}

/*------------------
Ampersand Checker
-------------------*/
int checkAmpersand(char** args){
    for( i = 0; args[i] != NULL; i++){
        if(args[i][0] == '&'){
            // free(args[i]);
            args[i] = NULL;
            // printArr(array);
            return 1;
        }
    }
    // printArr(array);

    return 0;
}
/*------------------
Background Execution
-------------------*/
void background_execute(int count){

    int pid = fork();
    childpid = pid;

    if (pid < 0)
    {
        printf("Error: Fork Failed\n");
        return;
    }
    else if (pid == 0)
    {
        setpgid(0, 0);
        execvp(array[0], array);
    }
    else
    {
        back_count++;
        printf("[%d] %d\n", back_count, pid);
    }

    char name[100];
    strcpy(name, array[0]);

    for (i = 1; array[i] != NULL; i++)
    {
        strcat(name, " ");
        strcat(name, array[i]);
    }

    back[back_count].status = STATUS_RUNNING;
    back[back_count].pid = pid;
    back[back_count].is_back = 1;
    strcpy(back[back_count].name, name);
}


/*------------------
Jobs Feature
-------------------*/
void print_jobs()
{
    int i;
    int j = 1;
    for (i = 1; i <= back_count; i++)
    {
        if (back[i].is_back == 1)
        {
            if (back[i].status == STATUS_DONE)
            {
                printf("[%d] %s                    %s\n", j, "Done", back[i].name);
            }
            else
            {
                if (back[i].status == STATUS_SUSPENDED){
                    printf("[%d] ", j);
                    printf("%s ", "Stopped                 ");
                    printf("%s \n", back[i].name);
                }
                else{
                    if(switcher == 0){
                        printf("[%d]- ", j);
                        switcher = 1;
                    }else{
                        printf("[%d]+ ", j);
                        switcher = 0;
                    }
                    printf("%s ", "Running                ");
                    printf("%s &\n", back[i].name);
                }
            }
            j++;
        }
    }
}

/*------------------
Front ground feature
-------------------*/

void fg(int k)
{
    int proc = atoi(&array[1][1]);
    if (proc > back_count)
        printf("No such job\n");
    else
    {
        pid_t pid = back[proc].pid;
        if (pid < 0)
        {
            printf("Process has been terminated. Cannot bring to foreground.\n");
        }
        else
        {
            printf("%s \n", back[proc].name);

            kill(pid, SIGCONT);
            childpid = pid;
            strcpy(fore.name, back[proc].name);
            fore.pid = back[proc].pid;
            fore.is_back = 0;
            int j = proc;
            for (j = proc; j < back_count; j++)
            {
                back[j] = back[j + 1];
            }
            back_count--;

            waitpid(-1, NULL, WUNTRACED);

        }
    }
}
/*------------------
Background Feature
-------------------*/
void bg(int k)
{ 
    int j = 1;

    int proc = atoi(&array[1][1]);
    if (proc > back_count)
        printf("No such job\n");
    else 
    {
        back[proc].status = STATUS_CONTINUED;
        printf("[%d]+ %s &\n",j,back[proc].name);
        j++;
        pid_t pid = back[proc].pid;
        kill(pid, SIGTTIN);
        kill(pid, SIGCONT);
    }

}
/*------------------
Clear array for next execution
-------------------*/
void clearArray(char** des){
    for(i = 0; des[i] != NULL; i++){
        des[i] = NULL;
    }
    printArr(array);
}
/*------------------
Color for reminder feature
-------------------*/
void red () {
  printf("\033[1;31m");
}
void yellow() {
  printf("\033[1;33m");
}
void resetcolor () {
  printf("\033[0m");
}



/*------------------
Additional Feature
-------------------*/
void remindmeto(int count)
{

    int rem = atoi(array[1]);
    int pid = fork();
    if (pid < 0)
        printf("Sorry reminder could not be added\n");
    else if (pid == 0)
    {
        sleep(rem);
        yellow();
        printf("\nREMINDER: ");
        printf("\n");
        for(i = 2; array[i] != NULL ; i++){
            printf("\n");
            red();
            printf("!! %s !!\n",array[i]);
            printf("\n");
        }
        resetcolor();
    }else{
        return;
    }

}



void resetArray(char** args){
    for(i = 0; args[i] != NULL ; i++){
        args[i] = NULL;
    }
}





int main(int argc, char const *argv[]){

    if(argv[1] != NULL){
        char str[MAXCHAR];

        fp = fopen(argv[1], "r");

        while (fgets(str, MAXCHAR, fp) != NULL){
            // printf("Print: %s \n", str);
            makeTokens(str);
            if (strcmp(array[0], "exit") == 0) {
                printf("Bye\n");
                return atoi(array[1]);
            }
            checkBangs(array,prev);
            int count = 0;
            for(i = 0; array[i] != NULL; i++){
                count++;
            }
            execute(count);
            
        }
        fclose(fp);     
    }

    else{
        shellid = getpid();

        sigaction(SIGINT, &(struct sigaction){ .sa_handler = sigintHandler }, NULL);
        signal(SIGTSTP, ctrl_z);
        signal(SIGCHLD, SIG_IGN);
        signal(SIGCHLD, child_sig);


        startDisplay();

        while(1){
            displayPrompt(); // Display a user prompt
            getline(&input, &capline, stdin); // Read the user input
            /* Check if input is empty */
            if(strcmp(input,"\n")==0){
                continue;
            }


            ofile = NULL;
            ifile = NULL;


            makeTokens(input); // Divide line into tokens

            int count = 0;

            /* Check if input is "q", if yes then exit shell */
            if (strcmp(array[0], "exit") == 0) {
                printf("Bye\n");
                return atoi(array[1]);
            }
            for(i = 0; array[i] != NULL; i++){
                count++;
            }
            set_count = count;
            block = checkAmpersand(array);

            checkRedirect(array,args2);

            checkBangs(array,prev);

 
            if(count == 1 && (strcmp(array[0], "jobs") == 0)){
                print_jobs();
            }
            else if(count == 2 && strcmp(array[0],"fg") == 0 && array[1][0] == '%'){
              fg(count);
            }else if (count == 2 && strcmp(array[0],"bg") == 0 && array[1][0] == '%'){
              bg(count);
            }else if( count >= 3 && (strcmp(array[0], "remindmeto") == 0)) {
                remindmeto(count);
            }else if(block == 1){
                    background_execute(count);
                    block = 0;
            }else{
                execute(count); // Call execvp()

            }
        }            

    }
}