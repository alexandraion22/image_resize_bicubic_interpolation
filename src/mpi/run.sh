#!/bin/sh

if [ $# -lt 2 ]
then
    echo "Numar gresit de argumente. Script-ul se utilizeaza astfel: ./run.sh <factor> <numar_procese>"
else
    make
    mpirun -np "$2" ./resize Kittens.png "$1"
    make clean
fi