#!/bin/bash
gcc -o db-connect -std=c99 -I/usr/include/mysql db-connect.c $(mysql_config --libs)
./db-connect
