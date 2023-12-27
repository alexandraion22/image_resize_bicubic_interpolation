#!/bin/sh

if [ $# -lt 2 ]
then
    echo "Numar gresit de argumente. Script-ul se utilizeaza astfel: ./run.sh <factor> <numar_threaduri>"
else
    make
    export OMP_NUM_THREADS="$2"
    ./resize Kittens.png "$1"
    export OMP_NUM_THREADS=""
    make clean
fi