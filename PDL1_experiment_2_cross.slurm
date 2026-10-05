#!/bin/bash
#SBATCH --job-name=PDL1_experiment_2_cross
#SBATCH --output=logs/PDL1_experiment_2_cross/pdl1.2_cross_%A_%a.out
#SBATCH --error=logs/PDL1_experiment_2_cross/pdl1.2_cross_%A_%a.err
#SBATCH --partition=mcs.default.q,tue.default.q
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --time=03:15:00
#SBATCH --mem=32G

#SBATCH --array=0-539

C=(1 2 3)
YQ=(4 5 6)
D=(3)
MARGIN_WIDTH=(0 0.2 0.4 0.6 0.8 1.0)
SPLITS=(0 1 2 3 4 5 6 7 8 9)

NUM_MW=${#MARGIN_WIDTH[@]}
NUM_YQ=${#YQ[@]}
NUM_D=${#D[@]}
NUM_SPLITS=${#SPLITS[@]}

C_IDX=$(( SLURM_ARRAY_TASK_ID / (NUM_YQ * NUM_D * NUM_MW * NUM_SPLITS) ))
REM=$(( SLURM_ARRAY_TASK_ID % (NUM_YQ * NUM_D * NUM_MW * NUM_SPLITS) ))
YQ_IDX=$(( REM / (NUM_D * NUM_MW * NUM_SPLITS) ))
REM=$(( REM % (NUM_D * NUM_MW * NUM_SPLITS) ))
D_IDX=$(( REM / (NUM_MW * NUM_SPLITS) ))
REM=$(( REM % (NUM_MW * NUM_SPLITS) ))
MW_IDX=$(( REM / NUM_SPLITS ))
SPLIT_IDX=$(( REM % NUM_SPLITS ))

SELECTED_C=${C[$C_IDX]}
SELECTED_YQ=${YQ[$YQ_IDX]}
SELECTED_D=${D[$D_IDX]}
SELECTED_MW=${MARGIN_WIDTH[$MW_IDX]}
SELECTED_SPLIT=${SPLITS[$SPLIT_IDX]}

source hpc/load_modules.sh

echo "Starting task ${SLURM_ARRAY_TASK_ID}: C=${SELECTED_C}, YQ=${SELECTED_YQ}, D = ${SELECTED_D} MARGIN_WIDTH=${SELECTED_MW}, SPLIT=${SELECTED_SPLIT}"

bash scripts/PDL1_experiment_2_cross/experiment.sh "$SELECTED_YQ" "$SELECTED_MW" "$SELECTED_C" "$SELECTED_D" "$SELECTED_SPLIT"

echo "Job finished at $(date)"
