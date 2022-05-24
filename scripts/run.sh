#!/bin/bash
./benchmark.sh neurons 5
git add -f neurons.csv
git commit -m "Neurons test done"
git push

./benchmark.sh density 5
git add -f density.csv
git commit -m "Density test done"
git push
