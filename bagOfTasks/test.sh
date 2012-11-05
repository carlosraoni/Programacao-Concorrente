#!/bin/sh

NEXEC=$1
NWORKERS=$2
NTASKS=$3
RINI=$4
REND=$5

totalTime1=0
totalTime2=0
totalTime3=0
totalTime4=0

#for i in `seq 1 $NEXEC`; do
#	echo "Run $i -------------------"

	time1=`solution1.exe $NWORKERS $RINI $REND | grep Time | cut -d ' ' -f 4`
#	totalTime1=`echo $totalTime1+$time | bc -l`
	echo "t1 = $time1"

	time2=`solution2.exe $NWORKERS $NTASKS $RINI $REND | grep Time | cut -d ' ' -f 4`
#	totalTime2=`echo $totalTime2+$time | bc -l`
	echo "t2 = $time2"

	time3=`solution3.exe $NWORKERS $NTASKS $RINI $REND | grep Time | cut -d ' ' -f 4`
#	totalTime3=`echo $totalTime3+$time | bc -l`
	echo "t3 = $time3"

	time4=`solution4.exe $NWORKERS $NTASKS $RINI $REND | grep Time | cut -d ' ' -f 4`
#	totalTime4=`echo $totalTime4+$time | bc -l`
	echo "t4 = $time4"
#done

#meanTime1=`echo "scale=3; $totalTime1/$NEXEC" | bc -l`
#meanTime2=`echo "scale=3; $totalTime2/$NEXEC" | bc -l`
#meanTime3=`echo "scale=3; $totalTime3/$NEXEC" | bc -l`
#meanTime4=`echo "scale=3; $totalTime4/$NEXEC" | bc -l`

#echo "TotalTime = $totalTime"
echo "MeanTime = $time1 $time2 $time3 $time4"

