#!/bin/bash

MAX=0;
for file in $( ls ../tests/density ); do
	LEN=$( echo -n $file | sed 's/.*_\(.*\)\..*\.rpl/\1/' | wc -c ); 
	if [ $LEN -gt $MAX ]; then 
		MAX=$LEN; 
	fi; 
done;

for file in $( ls ../tests/density ); do
	LEN=$( echo -n $file | sed 's/.*_\(.*\)\..*\.rpl/\1/' | wc -c );
	LEN_PAD=$(( MAX - LEN ))
	PADDING=""
	for i in $( seq $LEN_PAD ); do
		PADDING=0$PADDING
	done
	if [ $LEN_PAD -ne 0 ]; then
		mv ../tests/density/$file ../tests/density/$( echo $file | sed "s/\(.*\)_\(.*\)/\1_$PADDING\2/" )
	fi
done
