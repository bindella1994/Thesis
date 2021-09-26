
ll#!/bin/bash
DIR="dataTest"
if [ -d "$DIR" ]; then
  # Take action if $DIR exists. #
  echo "${DIR} yet existent..."
else
  mkdir $DIR
fi

./startMeanAcceleration
sleep 1
./startComparationTest

mv result.csv resultCpp.csv dataTest
