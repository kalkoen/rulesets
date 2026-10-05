#!/bin/bash
#SBATCH --job-name=PDL1_experiment_1
#SBATCH --output=logs/PDL1_experiment_1/pdl1.1_%A_%a.out  # %A is JobID, %a is Array Index
#SBATCH --error=logs/PDL1_experiment_1/pdl1.1_%A_%a.err
#SBATCH --partition=mcs.default.q,tue.default.q
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --time=03:05:00
#SBATCH --mem=2G


# --- JOB ARRAY CONFIGURATION ---
# Example: If you have 3 datasets and want to run indices 1 to 5 for each:
# Total tasks = (number of names) * (number of indices)
#SBATCH --array=0-44

mkdir -p logs/PDL1_experiment_1/


# 1. Define your list of names
NQ=(1 3 5 7 9)
NUM_NQS=${#NQ[@]}

K=9
C_VAL=20

# 3. Calculate which name and index this specific task should handle
# Task 0 -> Name 0, Index 1
# Task 1 -> Name 0, Index 2
# Task 2 -> Name 1, Index 1...
NQ_IDX=$(( SLURM_ARRAY_TASK_ID / K ))
Q=$(( (SLURM_ARRAY_TASK_ID % K) ))
SELECTED_NQ=${NQ[$NQ_IDX]}


SETTINGS="settings.set"

# Load and Run
source hpc/load_modules.sh

echo "Starting task ${SLURM_ARRAY_TASK_ID}: XNQ=${SELECTED_NQ}, YQ=${Q}"

# Execute (using 'bash' is safer than 'source' here to avoid env pollution)
bash scripts/PDL1_experiment_1/experiment.sh "$SELECTED_NQ" "$Q" "$C_VAL" "$SETTINGS"

echo "Job finished at $(date)"
