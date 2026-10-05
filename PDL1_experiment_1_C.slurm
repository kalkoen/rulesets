#!/bin/bash
#SBATCH --job-name=PDL1_experiment_1_C
#SBATCH --output=logs/PDL1_experiment_1_C/pdl1.1_%A_%a.out
#SBATCH --error=logs/PDL1_experiment_1_C/pdl1.1_%A_%a.err
#SBATCH --partition=mcs.default.q,tue.default.q
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --time=3:05:00
#SBATCH --mem=2G

# --- JOB ARRAY CONFIGURATION ---
#SBATCH --array=0-519

mkdir -p logs/PDL1_experiment_1_C/

# --- 1. DEFINE YOUR ARRAYS ---
XNQ_ARRAY=(1 9)
YQ_ARRAY=(4)
S_ARRAY=(0 1 2 3 4 5 6 7 8 9)
C_ARRAY=({5..30})

# --- 2. GET LENGTHS ---
L_XNQ=${#XNQ_ARRAY[@]}
L_YQ=${#YQ_ARRAY[@]}
L_S=${#S_ARRAY[@]}
L_C=${#C_ARRAY[@]}

# --- 3. DECODE THE QUADRUPLE INDEX ---
# We decode from right to left: S -> YQ -> C -> XNQ (or any preferred order)
S_IDX=$(( SLURM_ARRAY_TASK_ID % L_S ))
YQ_IDX=$(( (SLURM_ARRAY_TASK_ID / L_S) % L_YQ ))
C_IDX=$(( (SLURM_ARRAY_TASK_ID / (L_S * L_YQ)) % L_C ))
XNQ_IDX=$(( SLURM_ARRAY_TASK_ID / (L_S * L_YQ * L_C) ))

SELECTED_XNQ=${XNQ_ARRAY[$XNQ_IDX]}
SELECTED_YQ=${YQ_ARRAY[$YQ_IDX]}
SELECTED_S=${S_ARRAY[$S_IDX]}
SELECTED_C=${C_ARRAY[$C_IDX]}

SETTINGS="settings.set"

source hpc/load_modules.sh

echo "Starting task ${SLURM_ARRAY_TASK_ID}: XNQ=${SELECTED_XNQ}, YQ=${SELECTED_YQ}, S=${SELECTED_S}, C=${SELECTED_C}"

# Updated command to include the 4th variable
bash scripts/PDL1_experiment_1_C/experiment.sh "$SELECTED_YQ" "$SELECTED_S" "$SELECTED_XNQ" "$SELECTED_C" "$SETTINGS"

echo "Job finished at $(date)"