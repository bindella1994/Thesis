#!/bin/bash
FILE="scrubbingTestCorruzioniParziali.csv"
if [ -f "$FILE" ]; then
    rm $FILE
fi
echo "Index,percentage">> $FILE
for (( i = 0 ; i < 201; i = i + 10))
do
	result=$( ./scrubbingTestCorruzioniParziali 1000 $i )
	echo "$i,$result" >> $FILE
	echo "Completato per $i: $result"
	#sleep 1
done

