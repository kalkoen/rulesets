import os

import pandas as pd

from data import dump_splits

random_state = 42

n_splits = 10
experiment_name = "PDL1_experiment_3_cross"
folder_out_data = f"data/{experiment_name}/split"
os.makedirs(folder_out_data, exist_ok=True)

folder_read_data = f"data/PDL1_experiment_3"
quantiles = range(9)

for folder in [folder_out_data]:
    if not os.path.exists(folder):
        os.makedirs(folder)
 
X = pd.read_csv(folder_read_data + "/X_broad.csv")
for q in quantiles:
    y = pd.read_csv(folder_read_data + f"/y/y_broad_q_{q}.csv")
    print(y)
    dump_splits(X,
                y,
                n_splits,
                folder_out_data,
                f"y_q_{q}_s",
                random_state=random_state)