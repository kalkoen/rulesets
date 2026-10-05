import os

from ucimlrepo import fetch_ucirepo
import numpy as np
from sklearn.model_selection import StratifiedKFold
import pandas as pd

from scripts.binarizer import *
from scripts.data import dump_splits

random_state = 42

n_quantile_set_X = [1, 3, 5, 7, 9]
n_quantiles_y = 9
n_splits = 10

experiment_name = "PDL1_experiment_1_cross"
experiment_data_folder = f"data/{experiment_name}"

folder_out_split = f"{experiment_data_folder}/split/"
folder_out_bin = f"{experiment_data_folder}/binarized/"
folder_out_results = f"{experiment_data_folder}/results/"
out_settings = f"{experiment_data_folder}/settings.set"

file_X = "data/X_TFactivities_Broad.csv"
file_y = "data/Y_PDL1expression_Broad.csv"

for folder in [folder_out_split, folder_out_bin, folder_out_results]:
    # assert not os.path.exists(folder), "You must first remove all relevant folders"
    os.makedirs(folder, exist_ok=True)


X = pd.read_csv(file_X)
X = X.pivot(values="TF activity score", index="sample", columns=["gene"])

y = pd.read_csv(file_y).set_index("sample")

y_bin, [] = binarize_data(y, [], n_quantiles_y, False, decimals=4)

for i, column in enumerate(y_bin):
    yq = y_bin[column]
    train_splits, test_splits = dump_splits(X,
                yq,
                n_splits,
                folder_out_split,
                identifier=f"y_q_{i}_s",
                random_state=random_state)

    for (dname_train, (X_train, y_train)), (dname_test, (X_test, y_test)) \
            in zip(train_splits.items(), test_splits.items()):

        for xnq in n_quantile_set_X:
            X_bin_train, [X_bin_test] = binarize_data(X_train, [X_test], xnq, True, 2)

            X_bin_train.astype(int).to_csv(f"{folder_out_bin}/{dname_train}_X_nq_{xnq}_train_X.csv", index=False)
            X_bin_test.astype(int).to_csv(f"{folder_out_bin}/{dname_train}_X_nq_{xnq}_test_X.csv", index=False)

        y_train.astype(int).to_csv(f"{folder_out_bin}/{dname_test}_train_y.csv", index=False)
        y_test.astype(int).to_csv(f"{folder_out_bin}/{dname_test}_test_y.csv", index=False)


# for (dname_train, (X_train, y_train)), (dname_test, (X_test, y_test)) \
#         in zip(train_splits.items(), test_splits.items()):
#     X_bin_train, [X_bin_test] = binarize_data(X_train, [X_test], n_quantiles, True, 2)
#
#     X_bin_train.astype(int).to_csv(f"{folder_out_bin}/{dname_train}_train_X.csv", index=False)
#     X_bin_test.astype(int).to_csv(f"{folder_out_bin}/{dname_train}_test_X.csv", index=False)
#     y_train.astype(int).to_csv(f"{folder_out_bin}/{dname_test}_train_y.csv", index=False)
#     y_test.astype(int).to_csv(f"{folder_out_bin}/{dname_test}_test_y.csv", index=False)
#
# for n_quantiles in n_quantile_set_X:
#     X_bin, [] = binarize_data(X, [], n_quantiles, False)
#     X_bin.astype(int).to_csv(os.path.join(folder_out_bin, f"X_nq_{n_quantiles}.csv"), index=False)
#
# y_bin, [] = binarize_data(y, [], n_quantiles_y, False, decimals=4)
# for i, column in enumerate(y_bin):
#     y_bin[column].astype(int).to_csv(os.path.join(folder_out_bin, f"y_q_{i}.csv"), index=False)
