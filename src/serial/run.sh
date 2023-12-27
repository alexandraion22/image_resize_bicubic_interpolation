#!/bin/sh

if [ $# -lt 1 ]
then
    echo "Numar gresit de argumente. Script-ul se utilizeaza astfel: ./run.sh <factor>"
else
    make
    ./resize Kittens.png "$1"
    make clean
fi