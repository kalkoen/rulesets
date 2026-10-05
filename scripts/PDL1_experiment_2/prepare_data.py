import os

import pandas as pd

from binarizer import binarize_data

n_quantiles_y = 9
experiment_name = "PDL1_experiment_2"
folder_out_data = f"data/{experiment_name}/"

for folder in [folder_out_data]:
    if not os.path.exists(folder):
        os.makedirs(folder)

for dataset in ["Broad", "Sanger"]:
    file_X = f"data/X_TFactivities_{dataset}.csv"
    file_y = f"data/Y_PDL1expression_{dataset}.csv"

    X = pd.read_csv(file_X)
    y = pd.read_csv(file_y).set_index("sample")

    X_pivot = X.pivot(values="TF activity score", index="sample", columns=["gene"])
    final = X_pivot

    if dataset == "Sanger":
        folder_out_data = os.path.join(folder_out_data, "sanger")
        if not os.path.exists(folder_out_data):
            os.makedirs(folder_out_data)

    final.to_csv(folder_out_data + "/X.csv", index=False)
    final.to_csv(folder_out_data + "/X_with_sample.csv", index=True)

    y.to_csv(folder_out_data + "/y.csv", index=False)
    y.to_csv(folder_out_data + "/y_with_sample.csv", index=True)


    y_bin, [] = binarize_data(y, [], n_quantiles_y, False, decimals=4)
    print(y_bin)
    for i, column in enumerate(y_bin):
        y_bin[column].astype(int).to_csv(os.path.join(folder_out_data, f"y_q_{i}.csv"), index=False)
        y_bin[column].astype(int).to_csv(os.path.join(folder_out_data, f"y_q_{i}_with_sample.csv"), index=True)