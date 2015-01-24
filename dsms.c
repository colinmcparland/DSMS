/******************************************************
*
* This program takes data from a file when prompted by an interrupt.  It also takes in user SQL queries to manipulate
*  the data in question.
*
*******************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h> 

/*********************************
*
*  Implement a signal handler to react once SIGUSR1 comes through
*
*********************************/

void my_handler(int signum)
{
	if(signum == SIGUSR1)
	{
		printf("Signal received.\n");
	
		/*********************************
		*
		*  Now that the signal has been received, go get a line from the shared file.
		*
		*********************************/
	
		//declare some variables to store a line from the input, 
		char *line = (char *)malloc(100);
	
		//open the shared file up for reading.  Used manual name instead of argv[2] since not accessible in sig handler
		FILE *fp = fopen("outputfile.txt", "r");
	
		//read a line from the shared file
		fgets(line, 100, fp);
		printf("%s\n", line);

		//close the file
		fclose(fp);

		//open and close the shared file to clear it and] make room for new data
		fp = fopen("outputfile.txt", "w");
		fclose(fp);


	}
}

int main(int argc, char *argv[]){

	/*********************************
	*
	*  First, we want to compose the arguments that need to be passed into our data stream process.  We will start by parsing the config file.
	*
	*********************************/

	//open up the config file
	FILE *fp1 = fopen(argv[1], "r");

	int i;
	char *dbname = (char *)malloc(256);
	char *initfile = (char *)malloc(256);
	char *usr1 = (char *)malloc(256);
	int clean;

	//store each line in a variable.
	for(i=0;i<4;i++)
	{
		switch(i)
		{
			case 0:
				fgets(dbname, 256, fp1);
			case 1:
				fgets(initfile, 256, fp1);
			case 2:
				fgets(usr1, 256, fp1);
			case 3:
				fscanf(fp1, "%d", &clean);

		}
	}

	/*********************************
	*
	*  Now, we want to run the database initialize code.
	*
	*********************************/

	
	//declare argument list for stream generator
	static char *argv2[] = {"./datagen", "5", "outputfile.txt", "SIGUSR1", "", "usrinput", NULL};

	//get current process ID, add it to the list
	static int curr;
	curr = getpid();
	argv2[4] = (char *)malloc(sizeof(curr));
	sprintf(argv2[4], "%d", (int)curr);

	/*********************************
	*
	*  Now, we want to fork this process to initialize a data stream.  When in the child process,
	*    execute the data stream with the argument list built above.  Once the process is running 
	*    successfully, make the parent wait for interrupts to arrive so it can start reading data.
	*
	*********************************/
	
	//register signal handler
	
	if (signal(SIGUSR1, my_handler) == SIG_ERR)
	{
		printf("Signal cannot be handled right now.\n");
	}

	//fork the parent process and create a child process for our data stream
	pid_t pid = fork();

	//in the child process
	if(pid == 0)
	{
		printf("Forking data stream process.\n\n");
		execv("./datagen",argv2);

	}//in the parent process
	else
	{
	  	while(1)
	  	{	
	  		sleep(1);
	  	}
	}

	printf("Exiting DSMS\n");
	
	return;
}