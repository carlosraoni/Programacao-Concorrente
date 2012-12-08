#!/bin/sh

NEXEC=$1
NWORKERS=$2
NTASKS=$3
RINI=$4
REND=$5

`./adaptive_quadrature.exe $NWORKERS $RINI $REND`
#time1=`./adaptive_quadrature.exe $NWORKERS $RINI $REND | grep Time | cut -d ' ' -f 4`
echo "t1 = $time1"


