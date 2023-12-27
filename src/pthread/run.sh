#!/bin/sh

if [ $# -lt 2 ]
then
    echo "Numar gresit de argumente. Script-ul se utilizeaza astfel: ./run.sh <factor> <numar_threaduri>"
else
    make
    ./resize Kittens.png "$1" "$2"
    make clean
fi