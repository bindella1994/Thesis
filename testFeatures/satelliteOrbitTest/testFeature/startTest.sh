#!/bin/bash
DIR="dataTest"
if [ -d "$DIR" ]; then
  # Take action if $DIR exists. #
  echo "${DIR} yet existent..."
else
  mkdir $DIR
fi

./startMeanAcceleration
sleep 2
./startComparationTest

cat result.csv
cat resultCpp.csv


mv result.csv resultCpp.csv dataTest
