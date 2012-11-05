#!/bin/bash

NEXEC=2
NWORKERS=("16" "32" "64")
NTASKS=("10" "100" "1000")
RINI=0
REND=15

for NW in ${NWORKERS[@]}; do
	for NT in ${NTASKS[@]}; do
		echo "./test.sh $NEXEC $NW $NT $RINI $REND | tee ./testResults/testResult-$NW-$NT.out"
		./test.sh $NEXEC $NW $NT $RINI $REND | tee ./testResults/testResult-$NW-$NT.out
	done
done



