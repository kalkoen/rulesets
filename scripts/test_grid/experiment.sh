#!/bin/bash
# scripts/benchmark/benchmark.sh

IDX=${1}
MW=${2}

cd data/test_grid
mkdir -p results_uncorrected

../../build/rs \
    "X_${IDX}.csv" \
    "y_${IDX}.csv" \
    100 \
    "results/O_${IDX}_mw_${MW}.json" \
    "settings.set" \
    "${MW}"

cd ../../