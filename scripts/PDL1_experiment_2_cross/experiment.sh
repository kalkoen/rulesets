#!/bin/bash
# scripts/PDL1_experiment_2/experiment.sh

YQ="${1}"
MARGIN_WIDTH="${2}"
C="${3}"
D="${4}"
SPLIT="${5}"

cd data/PDL1_experiment_2_cross || exit
mkdir -p results

../../build/rs \
    "split/y_q_${YQ}_s_${SPLIT}_train_X.csv" \
    "split/y_q_${YQ}_s_${SPLIT}_train_y.csv" \
    "${C}" \
    "results/yq_${YQ}_s_${SPLIT}_mw_${MARGIN_WIDTH}_C_${C}_D_${D}.json" \
    "../PDL1_experiment_2/settings_D_${D}.set" \
    "${MARGIN_WIDTH}"
