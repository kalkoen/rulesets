for yq in $(seq 0 8); do
  for mw in 0 0.2 0.4 0.6 0.8 1.0 1.5 2.0; do
    for C in $(seq 1 5); do
      bash scripts/PDL1_experiment_3/experiment.sh "$yq" "$mw" "$C" settings_D_1.set
    done
  done
done