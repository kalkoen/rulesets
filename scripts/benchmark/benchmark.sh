#!/bin/bash
# scripts/benchmark/benchmark.sh

NAME="${1}"
INDEX="${2}"
C="${3}"
SETTINGS="${4}"

# Navigate to data directory
cd data/benchmark || exit
mkdir -p results

# Run the executable
../../build/rs \
    binarized/${NAME}_${INDEX}_train_X.csv \
    binarized/${NAME}_${INDEX}_train_y.csv \
    "${C}" \
    results/${NAME}_${INDEX}.json \
    "${SETTINGS}"

cd ../..