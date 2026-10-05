#!/bin/bash
# scripts/PDL1_experiment_2/experiment.sh

YQ="${1}"
MARGIN_WIDTH="${2}"
C="${3}"
SETTINGS="${4}"

cd data/PDL1_experiment_2 || exit
mkdir -p results

../../build/rs \
    "X.csv" \
    "y_q_${YQ}.csv" \
    "${C}" \
    "results/yq_${YQ}_mw_${MARGIN_WIDTH}_C_${C}_${SETTINGS}.json" \
    "${SETTINGS}" \
    "${MARGIN_WIDTH}"
