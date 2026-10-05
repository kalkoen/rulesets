#!/bin/bash
# scripts/benchmark/benchmark.sh

YQ="${1}"
SPLIT="${2}"
XNQ="${3}"
C="${4}"
SETTINGS="${5}"

echo "---------------------------------------"
echo "Experiment Configuration:"
echo "  YQ:       $YQ"
echo "  SPLIT:    $SPLIT"
echo "  XNQ:      $XNQ"
echo "  C:        $C"
echo "  SETTINGS: $SETTINGS"
echo "---------------------------------------"

cd data/PDL1_experiment_1_cross
mkdir -p results

../../build/rs \
    binarized/y_q_${YQ}_s_${SPLIT}_X_nq_${XNQ}_train_X.csv \
    binarized/y_q_${YQ}_s_${SPLIT}_train_y.csv \
    "${C}" \
    results/y_q_${YQ}_s_${SPLIT}_X_nq_${XNQ}.json \
    "${SETTINGS}"
