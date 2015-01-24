/******************************************************
*
* This program populates the file with random data.  This data will be read by our managment system after we send it an interrupt indicating
*    there is data to read.
*
*******************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>


int main(int argc, char *argv[])
{
	/*********************************
	*
	*  First, we want to process the arguments.
	*  argv[1] = time in seconds between data deployment
	*  argv[2] = name of the output file for DSMS to read
	*  argv[3] = interrupt value to send to the DSMS to notify it that there is new data
	*  argv[4] = process id of the DSMS, which we will notify when data is ready
	*  argv[5] = name of user input file
	*
	*********************************/

	printf("Data stream init.\n\n");

	//get time between deployments	
	int t;
	sscanf(argv[1], "%d", &t);

	//get name of output file 
	char *outfile = argv[2];

	//parse interrupt value from string passed in from parent
	int intvalue;

	if (strcmp(argv[3], "SIGUSR1") == 0)
	{
		intvalue = SIGUSR1;
	}
	else if (strcmp(argv[3], "SIGUSR2") == 0)
	{
		intvalue = SIGUSR2;
	}

	//get paret process ID to send interrupt to
	int p;
	sscanf(argv[4], "%d", &p);

	//get name of user input file to read from
	char *userfile = argv[5];


	/*********************************
	*
	*  OK, so now we have the arguments processed.  Time to start populating the file!
	*
	*********************************/

	//declare new file pointer for shared file
	FILE *fp;

	//declare new file pointer for user input file, open it, and declare a variable to store each line
	FILE *fp1 = fopen(userfile, "r");
	char *temp = (char *)malloc(100);
	int r;

	//sleep for t seconds to wait for db init to finish...
	sleep(t);

	for(;;)
	{

		/*********************************
		*
		*  First, we want to take data from the user input file, line by line
		*
		*********************************/

		//open output file for appending
		fp = fopen(outfile, "a");

		//read a line from user input
		fgets(temp, 100, fp1);

		//write the user data to the shared file
		fprintf(fp, "%s", temp);

		//close the file
		fclose(fp);

		/********************************
		*
		*  Now, we want to send an interrupt to our managment system indicating there is data 
		*  ready in the file waiting to be read.
		*
		********************************/

		pid_t p2 = p;

		kill(p2, intvalue);

		//printf("Signal sent to %d.\n", p);


		//if there is no more user input to store to the shared file, wait 1 minute for buffer and then quit.
		if(feof(fp1))
		{
			sleep(60);
			break;
		}



		sleep(t);
	}

	return;
}