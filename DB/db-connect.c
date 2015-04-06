#include <my_global.h>
#include <mysql.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>

void finishErr(MYSQL* con);
void setup(MYSQL* con);
void addEntry(MYSQL* con);

int main(int argc, char** argv){
	/*
	1. Initiation of a connection handle structure
	2. Creation of a connection
	3. Execution of a query
	4. Closing of the connection
	*/
	MYSQL* con = mysql_init(NULL);
	if(con==NULL){
		fprintf(stderr,"%s\n",mysql_error(con));
		exit(1);
	}
	setup(con);
	if (mysql_query(con, "INSERT INTO PhotoObjects VALUES(1,1,\"hashValue\")")) {
      finishErr(con);
  	}

	mysql_close(con);
	exit(0);
}
void finishErr(MYSQL* con){
	fprintf(stderr,"%s\n",mysql_error(con));
	mysql_close(con);
	exit(1);
}
void setup(MYSQL* con){
	// mysql> CREATE USER catfuser@localhost IDENTIFIED BY 'catfuser';
	// GRANT ALL ON testdb.* to catfuser@localhost
	if(mysql_real_connect(con,"localhost","catfuser","catfuser","testdb",0,NULL,0)==NULL){
		finishErr(con);
	}
	/*
	if(mysql_query(con,"CREATE DATABASE testdb")){
		finishErr(con);
	}*/
	if(mysql_query(con,"DROP TABLE IF EXISTS PhotoObjects")){
		finishErr(con);
	}
	if(mysql_query(con,"CREATE TABLE PhotoObjects(ObjID INT,Version INT,PathHash VARCHAR(25))")){
		finishErr(con);
	}
}
