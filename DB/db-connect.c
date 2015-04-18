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
void processQuery(int sock);

int main(int argc, char** argv){

	/* SETTING UP DATABASE
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
	
	// Setting up Server
	int sockfd,newsockfd;
	char buffer[256];
	static const int SERV_PORT = 12345;
	struct sockaddr_in serv_addr,cli_addr;

	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0){
		perror("ERROR: Opening Socket");
		exit(1);
	}

	bzero((char*)&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(SERV_PORT);

	if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
		perror("ERROR: Binding Socket");
	}

	listen(sockfd,5);
	printf("Starting Service ...\n");

	while(1){
		newsockfd = accept(sockfd,(struct sockaddr*)&cli_addr,sizeof(cli_addr));
		if(newsockfd < 0){
			perror("ERROR: Accepting Connection");
			exit(1);
		}
		pid_t pid = fork();
		if (pid < 0){
			perror("ERROR: Forking");
			exit(1);
		}
		if (pid == 0){
			close(sockfd);
			processQuery(newsockfd);
			exit(0);
		}
		else{
			close(newsockfd);
		}
	}

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
void processQuery(int sock){
	int n;
	char buffer[256];
	bzero(buffer,256);

	n = read(sock,buffer,255);

	if(n < 0){
		perror("ERROR: Reading from socket");
		exit(1);
	}

	printf("Here is the query: %s\n",buffer);

}
