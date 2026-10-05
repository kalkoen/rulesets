#!/bin/bash
#SBATCH --job-name=PDL1_experiment_1_cross
#SBATCH --output=logs/PDL1_experiment_1_cross/pdl1.1_%A_%a.out  # %A is JobID, %a is Array Index
#SBATCH --error=logs/PDL1_experiment_1_cross/pdl1.1_%A_%a.err
#SBATCH --partition=mcs.default.q,tue.default.q
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --time=3:05:00
#SBATCH --mem=2G

# --- JOB ARRAY CONFIGURATION ---
# Example: If you have 3 datasets and want to run indices 1 to 5 for each:
# Total tasks = (number of names) * (number of indices)
#SBATCH --array=0-449

mkdir -p logs/PDL1_experiment_1_cross/


# --- 1. DEFINE YOUR ARRAYS ---
XNQ_ARRAY=(1 3 5 7 9)
YQ_ARRAY=(0 1 2 3 4 5 6 7 8)
S_ARRAY=(0 1 2 3 4 5 6 7 8 9)

# --- 2. GET LENGTHS ---
L_XNQ=${#XNQ_ARRAY[@]}
L_YQ=${#YQ_ARRAY[@]}
L_S=${#S_ARRAY[@]}

# --- 3. DECODE THE TRIPLE INDEX ---

S_IDX=$(( SLURM_ARRAY_TASK_ID % L_S ))
YQ_IDX=$(( (SLURM_ARRAY_TASK_ID / L_S) % L_YQ ))
XNQ_IDX=$(( SLURM_ARRAY_TASK_ID / (L_S * L_YQ) ))

SELECTED_XNQ=${XNQ_ARRAY[$XNQ_IDX]}
SELECTED_YQ=${YQ_ARRAY[$YQ_IDX]}
SELECTED_S=${S_ARRAY[$S_IDX]}

C_VAL=20
SETTINGS="settings.set"

source hpc/load_modules.sh

echo "Starting task ${SLURM_ARRAY_TASK_ID}: XNQ=${SELECTED_XNQ}, YQ=${SELECTED_YQ}, S=${SELECTED_S}"

bash scripts/PDL1_experiment_1_cross/experiment.sh "$SELECTED_YQ" "$SELECTED_S" "$SELECTED_XNQ" "$C_VAL" "$SETTINGS"

echo "Job finished at $(date)"
