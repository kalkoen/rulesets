#!/bin/bash
#SBATCH --job-name=PDL1_experiment_1_full
#SBATCH --output=logs/PDL1_experiment_1_full/pdl1.1_%A_%a.out  # %A is JobID, %a is Array Index
#SBATCH --error=logs/PDL1_experiment_1_full/pdl1.1_%A_%a.err
#SBATCH --partition=mcs.default.q,tue.default.q
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --time=02:30:00
#SBATCH --mem=64G


# --- JOB ARRAY CONFIGURATION ---
# Example: If you have 3 datasets and want to run indices 1 to 5 for each:
# Total tasks = (number of names) * (number of indices)
#SBATCH --array=18-19

mkdir -p logs/PDL1_experiment_1_full/

NQ=(1 3 5)
NUM_NQS=${#NQ[@]}

K=9
C_VAL=20

NQ_IDX=$(( SLURM_ARRAY_TASK_ID / K ))
Q=$(( (SLURM_ARRAY_TASK_ID % K) ))
SELECTED_NQ=${NQ[$NQ_IDX]}

SETTINGS="settings.set"

source hpc/load_modules.sh

echo "Starting task ${SLURM_ARRAY_TASK_ID}: XNQ=${SELECTED_NQ}, YQ=${Q}"

bash scripts/PDL1_experiment_1_full/experiment.sh "$SELECTED_NQ" "$Q" "$C_VAL" "$SETTINGS"

echo "Job finished at $(date)"
