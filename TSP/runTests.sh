#!/bin/bash

NWORKERS=("2" "4" "8" "16")
DEPTH=("1" "2" "3")
FILE="input\ulysses16.tsp"

./seq_bb_tsp.exe $FILE | tee ./results/testResultSeq.out
for NW in ${NWORKERS[@]}; do
	for D in ${DEPTH[@]}; do
		./parallel_bb_tsp_1.exe $FILE $D $NW  | tee ./results/testResultP1-$D-$NW.out1
		./parallel_bb_tsp_2.exe $FILE $D $NW  | tee ./results/testResultP2-$D-$NW.out2
	done
done



