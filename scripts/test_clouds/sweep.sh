idxs=(1 2)
Cs=(1 2 3)
mws=(0 0.5 1 2 3 4 5)
setts=("additive.set" "nonadditive.set")

cd data/test_clouds || exit
mkdir -p results

for set in "${setts[@]}"; do
  for idx in "${idxs[@]}"; do
    for C in "${Cs[@]}"; do
      for mw in "${mws[@]}"; do
        ../../build/rs \
        "X_${idx}.csv" \
        "y_${idx}.csv" \
        "${C}" \
        "results/O_${idx}_C_${C}_mw_${mw}_${set}.json" \
        "${set}" \
        "${mw}"
      done
    done
  done
done

cd ../..