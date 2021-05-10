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



	// count numbers of indices in prev array
	int count;

	char *token;
	char *array[512]; 
	// For previous command
	char *prev[512]; 



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
			// count++;

			if(strcmp(token,"!!") == 0) {
				continue;
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
   		int pid = fork(); // Create a new process
		if (pid != 0) { // If not successfully completed
				int s;
				waitpid(-1, &s, 0); // Wait for process termination
		} else {
			if(execvp(array[0], array) == -1){ // If returned -1 => something went wrong! If not then command successfully completed */
				perror("Wrong command"); // Display error message
				exit(errno);
			}				
		}
		// reset count to zero
		// count = 0;
   }

   void copyArr(char** des, char** src){

	   	for(int j = 0; j < count; j++){
	   		// printf("Copy : %s\n",src[j]);
	   		des[j] =  strdup(src[j]);
	   	}
	   	// printf("Des: ");
	   	// printf("\n");
	   	// printArr(des);
	   	// printf("\n");
	   	// printf("Src: ");
	   	// printf("\n");

	   	// printArr(src);

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
   		copyArr(history,argv);
   	} 
   }


	
	int main(int argc, char const *argv[]){
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
			// if (strcmp(array[0], "exit") == 0 && strcmp(array[1], "1") == 0) {
			// 	printf("bye \n");
			// 	return 0;
			// }
			// printf("History \n");
			// printArr(prev);
			checkBangs(array,prev);
			// printArr(prev);

			execute(); // Call execvp()

	}
	
	
}