// #Pariwat Huang 6180067
// Reference : 
// 			https://github.com/szholdiyarov/command-line-interpreter/blob/master/myshell.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>   // Definition for fork() and execve()
#include <errno.h>    // Definition for "error handling"
#include <sys/wait.h> // Definition for wait()

	/* Declarations for getline() */
	char *input = NULL;
	size_t capline = 0; // Capacity
	
	/* Declaration for strtok() */
	int i;
	char *token;
	char *array[512]; 
	// For previous command
	char *prev[512]; 

	
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
		token = strtok(input, "\n ");
			while (token != NULL) { 
				array[i++] = token; // Add tokens into the array
				token = strtok(NULL, "\n ");
			}
		// printf("%s\n", array );
		array[i] = NULL;
	}

	/* Execute a command */
   void execute(){
	   int pid = fork(); // Create a new process
			if (pid != 0) { // If not successfully completed
					int s;
					waitpid(-1, &s, 0); // Wait for process termination
			} else {
					if(strcmp(array[0], "!!") == 0) {
							if(array[i-2])
							printf("Hey \n");
					}
					else if(execvp(array[0], array) == -1){ // If returned -1 => something went wrong! If not then command successfully completed */
						perror("Wrong command"); // Display error message
						exit(errno);
					}				
			}
   }
   
	
	int main(){
		startDisplay();
		while(1){
			displayPrompt(); // Display a user prompt
			getline(&input, &capline, stdin); // Read the user input
			/* Check if input is empty */
			if(strcmp(input,"\n")==0){
				perror("Please type in a command " );
				continue;
			}
		
			makeTokens(input); // Divide line into tokens 

            /* Check if input is "q", if yes then exit shell */
			if (strcmp(array[0], "exit") == 0 && strcmp(array[1], "1") == 0) {
				printf("bye \n");
				return 0;
			}

			execute(); // Call execvp()
	}
	
	
}