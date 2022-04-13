#!/bin/bash

echo "starting counting ...."

FOUND_OPTIMAL=0
NOT_OPTIMAL=0
NUM_TESTS=100

for (( c=1; c<=$NUM_TESTS; c++ ))
do
  echo "Test $c"
  ./qlearningprogram
  RET_CODE=$?

  if [ $RET_CODE -eq 0 ]; then
    ((FOUND_OPTIMAL=FOUND_OPTIMAL+1))
  else
    ((NOT_OPTIMAL=NOT_OPTIMAL+1))
  fi
done

echo "Optimal count=$FOUND_OPTIMAL out of $NUM_TESTS"
