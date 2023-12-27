#!/bin/sh

if [ $# -lt 3 ]
then
    echo "Numar gresit de argumente. Script-ul se utilizeaza astfel: ./run.sh <factor> <numar_procese> <numar_threaduri_pe_proces>"
else
    make
    export OMP_NUM_THREADS="$3"
    mpirun -np "$2" ./resize Kittens.png "$1"
    export OMP_NUM_THREADS=""
    make clean
fi