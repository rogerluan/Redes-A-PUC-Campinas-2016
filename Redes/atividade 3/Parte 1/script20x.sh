#!/bin/bash

COUNT=0
# bash until loop
until [ $COUNT -gt 20 ]; do
./cSleep localhost 5000 &
let COUNT=COUNT+1
done