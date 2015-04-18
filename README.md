Install the MySQL C Development Library:
sudo apt-get install libmysqlclient-dev

For GNU C compiler, need to add -std=99 option
To compile:
gcc -o db-connect -std=c99 -I/usr/include/mysql db-connect.c $(mysql_config --libs)