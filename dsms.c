/******************************************************
*
* This program takes data from a file when prompted by an interrupt.  It also takes in user SQL queries to manipulate
*  the data in question and produce output.
*
*******************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h> 
#include <sqlite3.h>


/*********************************
*
*  Declare some global variables to use in the signal handler
*
*********************************/

char *usr1;  //shared file name
char *dbname;  //name of database
char *initfile;  //name of initialization file
int clean;  //number of seconds between DB cleanings

/*********************************
*
*  Implement a callback function for SQL to format output for the user.
*
*********************************/

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    
    NotUsed = 0;
    int i;
    
    for (i = 0; i < argc; i++) 
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    
    printf("\n");
    
    return 0;
}

/*********************************
*
*  Implement a signal handler to react once SIGUSR1, SIGALRM or SIGHUP happen/
*
*********************************/

void my_handler(int signum)
{
	//handle a sigusr1

	if(signum == SIGUSR1)
	{

		/*********************************
		*
		*  SIGUSR1 has been received, go get a line from the shared file.
		*
		*********************************/
	
		//declare some variables to store a line from the input, 
		char *line = (char *)malloc(100);
	
		//open the shared file up for reading.  Used manual name instead of argv[2] since not accessible in sig handler
		FILE *fp = fopen(usr1, "r");
	
		//read a line from the shared file
		fgets(line, 100, fp);
		//printf("%s\n", line);

		/*********************************
		*
		*  Line acquired.  Now we need to parse it and add it to our sqlite database.  
		*
		*********************************/	

		char *name;
		char *num;
		char *pos;
		char *token;

		//first lets parse the line using spaces as a delimiter.
		//get name
		token = strtok(line, " ");
		name = token;

		//get number
		token = strtok(NULL, " ");
		num = token;

		//get position
		token = strtok(NULL, " ");
		pos = token;

		//now add it to the database!
		sqlite3 *db;
		sqlite3_open(dbname, &db);

		//write an sql command
		char *sql;

		sql = sqlite3_mprintf("INSERT INTO TEAM (NAME, NUMBER, POSITION) VALUES ('%q', '%q', '%q')", name, num, pos);

		//execute the command
		sqlite3_exec(db, sql, 0, 0, 0);

		//close the database
		sqlite3_close(db);

		//close the file
		fclose(fp);

		//open and close the shared file to clear it and make room for new data
		fp = fopen(usr1, "w");

		//close the file
		fclose(fp);

		//free memory
		free(line);
	}
	else if (signum == SIGALRM)  //handle an alarm
	{
		/*********************************
		*
		*  Alarm received, now we clear the database and set the alarm again.
		*
		*********************************/	

		//open a new database pointer
		sqlite3 *db;
		sqlite3_open(dbname, &db);

		//write an sql command
		char *sql;

		sql = "DELETE FROM TEAM;";
		//execute the command
		sqlite3_exec(db, sql, 0, 0, 0);

		//close the database
		sqlite3_close(db);

		alarm(clean);

	}
	else if (signum == SIGHUP)
	{

		/*********************************
		*
		*  Hang up received, re-read config file.
		*
		*********************************/	

		//open up the config file
		FILE *fp1 = fopen(initfile, "r");

		int i;

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

		fclose(fp1);

	}
}


int main(int argc, char *argv[]){

	/*********************************
	*
	*  First, we want to compose the arguments that need 
	*  	to be passed into our data stream process.  We will start by parsing the config file.
	*
	*********************************/

	//open up the config file
	FILE *fp1 = fopen(argv[1], "r");

	//set some variables
	int i;
	dbname = (char *)malloc(256);
	initfile = (char *)malloc(256);
	usr1 = (char *)malloc(256);

	//store each line in a variable.
	for(i=0;i<4;i++)
	{
		switch(i)
		{
			case 0:
				fgets(dbname, 256, fp1);
				break;
			case 1:
				fgets(initfile, 256, fp1);
				break;
			case 2:
				fscanf(fp1, "%s", usr1);
			case 3:
				fscanf(fp1, "%d", &clean);
				break;

		}
	}


	/*********************************
	*
	*  Now, we want to run the database initialize code.
	*
	*********************************/

	//execute the db initializer.  First build an arg list.
	static char *argv3[] = {"./db_init", "", NULL};
	argv3[1] = (char *)malloc(sizeof(dbname));
	sprintf(argv3[1], "%s", dbname);

	//fork the DB initialztion process.
	pid_t pid = fork();
	if(pid == 0)
	{
		execv("./db_init", argv3);
	}


	/*********************************
	*
	*  Now, we want to run the data stream.  Lets build an argument list and execute it.
	*
	*********************************/


	//declare argument list for stream generator
	static char *argv2[] = {"./datagen", "5", "", "SIGUSR1", "", "usrinput", NULL};

	//get current process ID, add it to the arg list
	static int curr;
	curr = getpid();
	argv2[4] = (char *)malloc(sizeof(curr));
	sprintf(argv2[4], "%d", (int)curr);

	//put shared file name in arguments
	argv2[2] = (char *)malloc(sizeof(usr1));
	sprintf(argv2[2], "%s", usr1);


	/*********************************
	*
	*  Now, we want to register some interrupt handlers so we can set our alarm and read data from the stream.
	*
	*********************************/
	
	//register signal handler for SIGUSR1, SIGALRM and SIGHUP
	
	if (signal(SIGUSR1, my_handler) == SIG_ERR)
	{
		printf("Signal SIGUSR1 cannot be handled right now.\n\n");
	}

	if(signal(SIGALRM, my_handler) == SIG_ERR)
	{
		printf("Signal SIGALRM cannot be handled right now.\n\n");
	}

	if(signal(SIGHUP, my_handler) == SIG_ERR)
	{
		printf("Signal SIGHUP cannot be handled right now.\n\n");
	}

	/*********************************
	*
	*  Set the alarm to clean the database now that the interrupt handler is registerd
	*
	*********************************/

	alarm(clean);

	/*********************************
	*
	*  Now we will actually fork the current process to initialize the data stream.  We can use the argument list we built above (argv2).
	*
	*********************************/

	//fork the parent process and create a child process for our data stream
	pid = fork();

	//in the child process
	if(pid == 0)
	{
		execv("./datagen",argv2);

	}//in the parent process
	else
	{
		/*********************************
		*
		*  While we wait for interrupts to come, we can prompt the user for SQL queries.
		*
		*********************************/

		char *cmd;  //sql command holder
		cmd = malloc(256);
		int rc;  //error checking


		while(1)
		{
			printf("Please enter an SQL statement.\n");
			fgets(cmd, 256, stdin);

			//open a new database pointer
			sqlite3 *db;
			sqlite3_open(dbname, &db);

			rc = sqlite3_exec(db, cmd, callback, 0, 0);

   			if( rc != SQLITE_OK )
   			{
   				fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
   			}
   			else
   			{
      			
  			}

  			sqlite3_close(db);
		}

		free(cmd);

	}

	//free memory and close files and such
	free(argv2[4]);
	free(argv3[1]);
	free(usr1);
	free(initfile);
	free(dbname);




	printf("Exiting DSMS\n");

	
	return;
}