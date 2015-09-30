##This is Operating System Course Final Project
In this project, we use FUSE to implement a distribute filesystem like dropbox. It can keep monitoring files under one folder, once file is added, deleted, or modified. Filesystem will hashing and sync with database in the remote. Back-end will make decision to pull or push replicas from remote sever. 
We use libcurl to implement data transfer, which can keep five replicas in the same time. Which fullfill the requirement that distributed. 

Contributor:
Ze Chen: Oh, it's me. I worked on FUSE API, and libcurl. so Filesystem is monitored and files can be transfered. 
Jiajie Yang: Exchange student from HK, He worked hard with me to implement this FUSE API and help to debug database code in python. 
Natch: My very best friend, he helped us to build out the db enviroment and setup the db structure. 

Install the MySQL C Development Library:
sudo apt-get install libmysqlclient-dev

For GNU C compiler, need to add -std=99 option
To compile:
gcc -o db-connect -std=c99 -I/usr/include/mysql db-connect.c $(mysql_config --libs)
