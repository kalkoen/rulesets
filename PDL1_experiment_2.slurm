#!/bin/bash
#SBATCH --job-name=PDL1_experiment_2
#SBATCH --output=logs/PDL1_experiment_2/pdl1.2_%A_%a.out
#SBATCH --error=logs/PDL1_experiment_2/pdl1.2_%A_%a.err
#SBATCH --partition=mcs.default.q,tue.default.q
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --time=03:15:00
#SBATCH --mem=32G
#SBATCH --array=0-599

C=(1 2 3 4 5)
YQ=(4 5 6 7 8)
D=(2 3 4)
MARGIN_WIDTH=(0 0.2 0.4 0.6 0.8 1.0 1.5 2.0)

NUM_MW=${#MARGIN_WIDTH[@]}
NUM_YQ=${#YQ[@]}
NUM_D=${#D[@]}

C_IDX=$(( SLURM_ARRAY_TASK_ID / (NUM_YQ * NUM_D * NUM_MW) ))
REM=$(( SLURM_ARRAY_TASK_ID % (NUM_YQ * NUM_D * NUM_MW) ))
YQ_IDX=$(( REM / (NUM_D * NUM_MW) ))
REM=$(( REM % (NUM_D * NUM_MW) ))
D_IDX=$(( REM / NUM_MW ))
MW_IDX=$(( REM % NUM_MW ))

SELECTED_C=${C[$C_IDX]}
SELECTED_YQ=${YQ[$YQ_IDX]}
SELECTED_D=${D[$D_IDX]}
SELECTED_MW=${MARGIN_WIDTH[$MW_IDX]}
SETTINGS="settings_D_${SELECTED_D}.set"

source hpc/load_modules.sh
echo "Starting task ${SLURM_ARRAY_TASK_ID}: C=${SELECTED_C}, YQ=${SELECTED_YQ}, D=${SELECTED_D}, MARGIN_WIDTH=${SELECTED_MW}"
bash scripts/PDL1_experiment_2/experiment.sh "$SELECTED_YQ" "$SELECTED_MW" "$SELECTED_C" "$SETTINGS"
echo "Job finished at $(date)"
