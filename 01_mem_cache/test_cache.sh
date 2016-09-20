#!/bin/bash
echo "params: " $1 $2 $3 $4
for i in `seq 1 1 $1`; do
	echo "Attempt " $i
	echo "`date`"
	echo "Attempt " $i 1>&2
	for j in `seq $2 $3 $4`; do
		./cache $j
	done
done
