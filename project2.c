/**
 *
 * @author Michael Scholl
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <pthread.h>
#include "Bank.h"
#include "Bank.c"
#include "project2.h"

/* Could make this value larger if required for commands */
#define BUFFER 505

/* Set our command and token */
static char command[BUFFER];
static char *token;

static char worker_command[BUFFER];
static char *worker_token;

int request_number = 1;
int running = 1;
node *root;
node *last_node;
pthread_mutex_t list_mutex;
FILE *output_file;
account *accounts;
 
int main(int argc, char *argv[]) {

	char *prompt = "> ";
    int num_worker_threads;
    int num_accounts;
    char *file_name;
    int i, j;
    
    // Set the number of threads to the amount of workers needed
    //pthread_t threads[num_worker_threads];

    // Check that there is 4 arguments
    if (argc != 4) {
        printf("Run the code like \"./project2 <# worker threads> <# accounts> <output file>\"\n");
        return 0;
    }

    // Get args from command line (Number of worker threads needed)
    num_worker_threads = atoi(argv[1]);
    // Get args from command line (Number of accounts being used)
    num_accounts = atoi(argv[2]);
    // Get args from command line (Name of the file that holds the output of the program)
    file_name = (char *) malloc(sizeof(char *) * 128);
    
    pthread_t threads[num_worker_threads];
    
    // Create the output file
    strcpy(file_name, argv[3]);
    output_file = fopen(file_name, "w");
    
    // Check to make sure the user gave a valid name for the file
    if (output_file == NULL) {
        printf("- Error creating output file\n");
    }

    // Catch the error initializing the bank accounts
    if (!initialize_accounts(num_accounts)) {
        printf("- Error creating accounts.\n");
        return 0;
    }
    else {
    	// If we did not have trouble initializing the accounts then tell us how many we made
        printf("         # of created accounts is %d\n", num_accounts);
    }

    // Make a mutex that each account can use
    accounts = (int *) malloc(sizeof(account) * num_accounts);
    for (i = 1; i <= num_accounts; i++) {
        account acct;
        pthread_mutex_init(&acct.lock, NULL);
        acct.account_id = i;
        accounts[i - 1] = acct;
    }
    
    // Initialize worker threads
    for (j = 0; j < num_worker_threads; j++) {
        // Create a new thread
        pthread_create(&threads[j], 0, worker_routine, NULL);
        //printf("Making #d worker threads\n", num_worker_threads);
    }
    
    // Initialize the nodes before going into main loop
    root = NULL;
    last_node = NULL;
    
    // Initialize the list mutex
    pthread_mutex_init(&list_mutex, NULL);
    
    // The main loop
    while (running) {
    	// Display the shell prompt
		printf("%s", prompt);
		
		// Grab input from user prompt
		fgets(command, sizeof command, stdin);
		
		// Tokenize the command
		token = strtok(command, " \t\n()<>|&;"); // I also used this in my project 1
		
		//printf("%s\n", token);
		
		if (!token) {
			continue;
		}
		
		// Exit on End
		else if(!strcmp(token, "end")) {
			exit(0);
		}
		
        // Create a new request and get the current time
        request req;
        gettimeofday(&req.time_start, NULL);
        
        //printf("Just checking the token:%s\n", token);
        strcpy(req.command, token);
        req.request_id = request_number++;
        
        // Update the list with the most recent request
        pthread_mutex_lock(&list_mutex);
        
        // Set root node
        if (root == NULL) {
        	
            root = (node *) malloc(sizeof(node));
            root->next_node = NULL;
            root->req = req;
            
            last_node = root;
        }
        // Set the next node based off the previous
        else if (last_node->next_node == NULL) {
        	
            node *new_node;
            new_node = (node *) malloc(sizeof(node));
            new_node->req = req;
            new_node->next_node = NULL;
            
            last_node->next_node = new_node;
            last_node = new_node;
        }
        
        pthread_mutex_unlock(&list_mutex);
    }
    
    fclose(output_file);
    return 0;
}

// Run the worker
void worker_routine() {
	//printf("Made it to worker!!!");
	
    // This is to wait for user to enter information
    while (root != NULL || running == 1) {
    	
        pthread_mutex_lock(&list_mutex);
        request req;
        
        // We enter this if so that our root can be messed with and we can check/trans
        if (root != NULL) {
            req = root->req;
            root = root->next_node;
            pthread_mutex_unlock(&list_mutex);

            printf("ID %d\n", req.request_id);
			strcpy(worker_command, req.command);
			worker_token = strtok(worker_command, " \t\n()<>|&;");
			//printf("%s Worker Token", worker_token);
			
			// Into the check function
			if (!strcmp(worker_token, "check")) {
				
				int request_id = req.request_id;
				int account_id = atoi(command);
				struct timeval time_start = req.time_start;
				int balance;
				account acct;
				long s_time_sec, s_time_usec, e_time_sec, e_time_usec;
				
				acct = accounts[account_id - 1];
				pthread_mutex_lock(&acct.lock);
				balance = read_account(acct.account_id);
				pthread_mutex_unlock(&acct.lock);
				
				struct timeval time_end;
				gettimeofday(&time_end, NULL);
				s_time_sec = time_start.tv_sec;
				s_time_usec = time_start.tv_usec;
				e_time_sec = time_end.tv_sec;
				e_time_usec =  time_end.tv_usec;
				
				// Write everything we just did to a file
				fprintf(output_file, "%d BAL %d TIME %d.%05d %d.%05d\n", request_id, balance,
					s_time_sec, s_time_usec, e_time_sec, e_time_usec);
			}
			// Into the transaction function
			if (!strcmp(worker_token, "trans")) {
				
				printf("To be implemented");
				/*
				int request_id = req.request_id;
				int account_id = atoi(command);
				struct timeval time_start = req.time_start;
				int balance;
				account acct;
				long s_time_sec, s_time_usec, e_time_sec, e_time_usec;
				
				
				struct timeval time_end;
				gettimeofday(&time_end, NULL);
				s_time_sec = time_start.tv_sec;
				s_time_usec = time_start.tv_usec;
				e_time_sec = time_end.tv_sec;
				e_time_usec =  time_end.tv_usec;
				
				// Write everything we just did to a file
				fprintf(output_file, "%d BAL %d TIME %d.%05d %d.%05d\n", request_id, balance,
					s_time_sec, s_time_usec, e_time_sec, e_time_usec);*/
			}
			
			else {
				//printf("Didn't get the string again");
			}
        }
        else {
        	// Make sure to unlock the list evem if we don't do anything
            pthread_mutex_unlock(&list_mutex);
            
        }
    }
}
