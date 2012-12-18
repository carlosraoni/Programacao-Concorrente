#!/bin/bash

NWORKERS=("2" "4" "8" "16")
DEPTH=("1" "2" "3" "4")
FILE="input\ulysses16.tsp"

for D in ${DEPTH[@]}; do
    for NW in ${NWORKERS[@]}; do
        btf1=`cat testResultP1-$D-$NW.out1 | grep btf | cut -d : -f 2`
		btf2=`cat testResultP2-$D-$NW.out2 | grep btf | cut -d : -f 2`

		bsf1=`cat testResultP1-$D-$NW.out1 | grep bsf | cut -d : -f 2`
		bsf2=`cat testResultP2-$D-$NW.out2 | grep bsf | cut -d : -f 2`

		tet1=`cat testResultP1-$D-$NW.out1 | grep tet | cut -d : -f 2`
		tet2=`cat testResultP2-$D-$NW.out2 | grep tet | cut -d : -f 2`

		echo -e "$D\t$NW\t$btf1\t$btf2" >> btf.report
		echo -e "$D\t$NW\t$bsf1\t$bsf2" >> bsf.report
		echo -e "$D\t$NW\t$tet1\t$tet2" >> tet.report
	done
done
