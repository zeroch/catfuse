#!/bin/bash

DIR="/home/zechen/devx
     /home/zechen/devy
     /home/zechen/devz
     /home/zechen/devzz"


for mount_dir in $DIR ;
do

    ./replica $mount_dir  -o nonempty #statements
done
