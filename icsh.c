// #Pariwat Huang 6180067
// Reference : 
//          https://github.com/szholdiyarov/command-line-interpreter/blob/master/myshell.c
//          https://stackoverflow.com/questions/58361506/save-history-command-on-simple-shell-by-c-code
//          https://stackoverflow.com/questions/52939356/redirecting-i-o-in-a-custom-shell-program-written-in-c
//          http://people.cs.pitt.edu/~khalifa/cs449/spr07/Assigns/Assign4/myshell.c
//          https://github.com/hungys/mysh/blob/master/mysh.c
//          https://www.gnu.org/software/libc/manual/html_node/Initializing-the-Shell.html
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

struct job
{
char name[100];
int pid;
//int state;
int is_back;
};


//Setting up Job
struct job back[100];
struct job fore;
int back_count = 0, shellid = 0;
int mode;

 // Setting up Childpid
int childpid = -1;


 // Declarations for getline() 
char *input = NULL;
size_t capline = 0; // Capacity

 // Declaration for strtok() 
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
// int count;
// int prev_count;
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

 // Display some info to the user at the start 
void startDisplay(){
printf("\n");
printf("Starting IC Shell                                            \n");
printf("\n");
}

 // Print out "MY_SHELL" 
void displayPrompt(){
    printf("icsh $ ");
}

 // Divide input line into tokens 
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

 // Execute a command 
void execute(int k){
    int status;
    int pid = fork(); // Create a new process
    if (pid < 0) { // If not successfully completed
        printf("Error: Fork Failed\n");
        exit(0);
    } else if(pid == 0) {
        setpgid(0, 0);

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

        for (i = 1; i < (k - 1); i++)
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

void copyArr(char** des, char** src){
    //Reset the firs array
    for(i = 0; des[i] != NULL; i++){
        des[i] = NULL;
    }

    for(j = 0; src[j] != NULL; j++){
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
        copyArr(argv,history);
    }
}
else{
    // If the length is one then do something else 
    copyArr(history,argv);
}

}
void sigintHandler(int sig_num)
{
    write(1, "\n", 1);
}

void sigstopHandler(int sig_num){
    printf("\n");
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
        if (exit_status == 0)
            printf("\n%s with pid %d is done\n", back[i].name, back[i].pid);
        else
            printf("\n%s with pid %d exited with exit status %d\n", back[i].name, back[i].pid, exit_status);
        printf("\n");
        fflush(stdout);
        break;
    }
}
signal(SIGCHLD, child_sig);
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
        // free(args[i]);
        args[i] = NULL;
        // printArr(array);
        return 1;
    }
}
// printArr(array);

return 0;
}

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

    // print_processes_of_job()

    back[back_count].pid = pid;
    back[back_count].is_back = 1;
    strcpy(back[back_count].name, name);
}

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
        printf("[1]+ Stopped %s \n", back[back_count].name );
    }
    signal(SIGTSTP, ctrl_z);
}

void print_jobs()
{
int i;
int j = 1;
for (i = 1; i <= back_count; i++)
{
    if (back[i].is_back == 1)
    {
        char stat[1000];
        char status;
        int p;
        long unsigned mem;
        char str[10];
        sprintf(str, "%d", back[i].pid);

        strcpy(stat, "/proc/");
        strcat(stat, str);
        strcat(stat, "/stat");
        FILE *fd;
        if (back[i].pid < 0)
        {
            printf("[%d] %s %s [%d]\n", j, "Done", back[i].name, back[i].pid);
        }
        else
        {
            // fscanf(fd, "%d %*s %c %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %lu %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d", &p, &status, &mem);
            // fclose(fd);
            printf("[%d] ", j);
            if (status == 'T')
                printf("%s ", "Stopped");
            else 
                printf("%s ", "Running");
            printf("%s [%d]\n", back[i].name, back[i].pid);
        }
        j++;
    }
}
}

void fg(int k)
{
int proc = atoi(&array[1][1]);
// if (k >= 3)
//     printf("Too many arguments\n");
// else if (k <= 1)
//     printf("Too few arguments\n");
// else
// {
if (proc > back_count)
    printf("No such job\n");
else
{
    pid_t pid = back[proc].pid;
    char stat[1000];
    char status;
    int p;
    long unsigned mem;
    char str[10];
    sprintf(str, "%d", back[proc].pid);

    strcpy(stat, "/proc/");
    strcat(stat, str);
    strcat(stat, "/stat");
    FILE *fd;
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

void bg(int k)
{

int proc = atoi(&array[1][1]);
if (proc > back_count)
    printf("No such job\n");
else 
{
    printf("Process continue to %s\n",back[proc].name);

    pid_t pid = back[proc].pid;
    kill(pid, SIGTTIN);
    kill(pid, SIGCONT);
}

}
// }
// }

//Check foreground parser "%"



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

            makeTokens(input); // Divide line into tokens 

            /* Check if input is "q", if yes then exit shell */
            if (strcmp(array[0], "exit") == 0) {
                printf("Bye\n");
                return atoi(array[1]);
            }
            checkRedirect(array,args2);

            int count = 0;
            for(i = 0; array[i] != NULL; i++){
                count++;
            }
            block = checkAmpersand(array);
            // //Checking for redirection
            // ofile = NULL;
            // ifile = NULL;
            // printf("Before\n");
            // printArr(array);
            // printf("After\n");
            // printArr(array);
            checkBangs(array,prev);
            if(count == 1 && (strcmp(array[0], "jobs") == 0)){
                print_jobs();
            }


            // printf("Prev\n");
            // printArr(prev);
            // fg_parser(array);


            if(count == 2 && strcmp(array[0],"fg") == 0 && array[1][0] == '%'){
              fg(count);
            }else if (count == 2 && strcmp(array[0],"bg") == 0 && array[1][0] == '%'){
              bg(count);
            }
            else{
                if(block == 1){
                    background_execute(count);
                    block = 0;
                }else{
                    execute(count); // Call execvp()
                }
            }

            
            //If it's not a background process/ else
            

        }
    // }
    }
}


// } 