#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	//open a database pointer, a name string and an error check integer
	sqlite3 *db;
	char *dbname = argv[1];
	int rc;
	//if return==1, error opening database...
	rc = sqlite3_open(dbname, &db);
	if (rc) //check if database connection was successful
	{
		printf("Error connecting to database.  %s\n", sqlite3_errmsg(db));
	}
	else//if so, init the database
	{

		char *sql;

		sql = "DELETE FROM TEAM;";

		sqlite3_exec(db, sql, 0, 0, 0);

		sql = "CREATE TABLE TEAM("\
			"Name varchar(255) NOT NULL,"\
			"Number int NOT NULL,"\
			"Position varchar(255) NOT NULL);";
		//execute the sql
		sqlite3_exec(db, sql, 0, 0, 0);

  		sqlite3_close(db);

  		}

  	return;
}

