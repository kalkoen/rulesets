#!/bin/bash
#SBATCH --job-name=PDL1_experiment_1_D
#SBATCH --output=logs/PDL1_experiment_1_D/pdl1.1_%A_%a.out  # %A is JobID, %a is Array Index
#SBATCH --error=logs/PDL1_experiment_1_D/pdl1.1_%A_%a.err
#SBATCH --partition=mcs.default.q,tue.default.q
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --time=3:05:00
#SBATCH --mem=2G


# --- JOB ARRAY CONFIGURATION ---
# Example: If you have 3 datasets and want to run indices 1 to 5 for each:
# Total tasks = (number of names) * (number of indices)
#SBATCH --array=1-9

mkdir -p logs/PDL1_experiment_1_D/

D=$SLURM_ARRAY_TASK_ID
XNQ=9
YQ=4
C_VAL=20

SETTINGS="settings_D_${D}.set"

source hpc/load_modules.sh

echo "Starting task ${SLURM_ARRAY_TASK_ID}: XNQ=${XNQ}, YQ=${YQ} D=${D} C_VAL=${C_VAL} SETTINGS=${SETTINGS}"

bash scripts/PDL1_experiment_1_D/experiment.sh "$XNQ" "$YQ" "$C_VAL" "$SETTINGS"

echo "Job finished at $(date)"
