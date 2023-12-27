#!/bin/sh

if [ $# -lt 3 ]
then
    echo "Numar gresit de argumente. Script-ul se utilizeaza astfel: ./run.sh <factor> <numar_procese> <numar_threaduri_pe_proces>"
else
    make
    mpirun -np "$2" ./resize Kittens.png "$1" "$3"
    make clean
fi