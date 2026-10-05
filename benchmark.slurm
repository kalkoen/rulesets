#!/bin/bash
#SBATCH --job-name=benchmark
#SBATCH --output=logs/benchmark/bench_%A_%a.out  # %A is JobID, %a is Array Index
#SBATCH --error=logs/benchmark/bench_%A_%a.err
#SBATCH --partition=mcs.default.q,tue.default.q
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --time=03:05:00
#SBATCH --mem=2G

# --- JOB ARRAY CONFIGURATION ---
# Example: If you have 3 datasets and want to run indices 1 to 5 for each:
# Total tasks = (number of names) * (number of indices)
#SBATCH --array=0-69

# 1. Define your list of names
NAMES=("banknote" "heart" "ILPD" "ionosphere" "tic-tac-toe" "transfusion" "WDBC")
C_VALS=(25 12 11 13 32 6 14)
NUM_NAMES=${#NAMES[@]}

# 2. Define how many indices (K) per dataset
K=10

# 3. Calculate which name and index this specific task should handle
# Task 0 -> Name 0, Index 1
# Task 1 -> Name 0, Index 2
# Task 2 -> Name 1, Index 1...
NAME_IDX=$(( SLURM_ARRAY_TASK_ID / K ))
DATA_INDEX=$(( (SLURM_ARRAY_TASK_ID % K) ))
SELECTED_NAME=${NAMES[$NAME_IDX]}

# 4. Set your other parameters
C_VAL=${C_VALS[$NAME_IDX]}
SETTINGS="settings.set"

# Load and Run
source hpc/load_modules.sh

echo "Starting task ${SLURM_ARRAY_TASK_ID}: Name=${SELECTED_NAME}, Index=${DATA_INDEX}"

# Execute (using 'bash' is safer than 'source' here to avoid env pollution)
bash scripts/benchmark/benchmark.sh "$SELECTED_NAME" "$DATA_INDEX" "$C_VAL" "$SETTINGS"

echo "Job finished at $(date)"
