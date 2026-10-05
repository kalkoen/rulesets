#!/bin/bash
# scripts/benchmark/benchmark.sh

XNQ="${1}"
YQ="${2}"
C="${3}"
SETTINGS="${4}"

mkdir -p data/PDL1_experiment_1_D
cd data/PDL1_experiment_1_D
mkdir -p results

../../build/rs \
    ../PDL1_experiment_1/binarized/X_nq_${XNQ}_neg.csv \
    ../PDL1_experiment_1/binarized/y_q_${YQ}.csv \
    "${C}" \
    results/Xnq_${XNQ}_yq_${YQ}_${SETTINGS}.json \
    "${SETTINGS}"
