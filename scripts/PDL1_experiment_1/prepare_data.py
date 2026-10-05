import os

from ucimlrepo import fetch_ucirepo
import numpy as np
from sklearn.model_selection import StratifiedKFold
import pandas as pd

from binarizer import *

from binarizer import binarize_data

n_quantile_set_X = [1, 3, 5, 7, 9]
n_quantiles_y = 9

folder_out_bin = "data/PDL1_experiment_1/binarized"
folder_out_results = "data/PDL1_experiment_1/results_uncorrected"
out_settings = "data/PDL1_experiment_1/settings_D_2.set"

file_X = "data/X_TFactivities_Broad.csv"
file_y = "data/Y_PDL1expression_Broad.csv"

for folder in [folder_out_bin, folder_out_results]:
    if not os.path.exists(folder):
        os.makedirs(folder)

X = pd.read_csv(file_X)
X = X.pivot(values="TF activity score", index="sample", columns=["gene"])

y = pd.read_csv(file_y)[["PDL1 expression"]]

for n_quantiles in n_quantile_set_X:
    X_bin, [] = binarize_data(X, [], n_quantiles, False)
    X_bin.astype(int).to_csv(os.path.join(folder_out_bin, f"X_nq_{n_quantiles}.csv"), index=False)

    X_bin_neg, [] = binarize_data(X, [], n_quantiles, True)
    X_bin_neg.astype(int).to_csv(os.path.join(folder_out_bin, f"X_nq_{n_quantiles}_neg.csv"), index=False)

y_bin, [] = binarize_data(y, [], n_quantiles_y, False, decimals=4)
for i, column in enumerate(y_bin):
    y_bin[column].astype(int).to_csv(os.path.join(folder_out_bin, f"y_q_{i}.csv"), index=False)
