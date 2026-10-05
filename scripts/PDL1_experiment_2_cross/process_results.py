import itertools

import numpy as np
import pandas as pd
import seaborn as sns
from matplotlib import pyplot as plt
from tqdm import tqdm

from results import *


def process_pdl1_experiment_2_cross(experiment_name,
                              result_file_pattern=
                              r"yq_{yq}_s_{s}_mw_{mw}_C_{C}_D_{D}.json",
                              X_train_filename_format="y_q_{yq}_s_{s}_train_X.csv",
                              y_train_filename_format="y_q_{yq}_s_{s}_train_y.csv",
                              X_test_filename_format="y_q_{yq}_s_{s}_test_X.csv",
                              y_test_filename_format="y_q_{yq}_s_{s}_test_y.csv"
                              ):
    data_folder = f"data/{experiment_name}/split"
    target_folder = f"data/{experiment_name}/results"

    columns = {
        "yq": int,
        "mw": float,
        "s": int,
        "C": int,
        "D": int
    }

    results = get_results_from_folder(target_folder, result_file_pattern, columns)
    results_df = get_results_df(results, columns)

    rows = []
    for idx, result, X, y, y_hat, X_test, y_test, y_test_hat in iterator_X_y_pred_train_test(
            results,
            data_folder,
            X_train_filename_format,
            y_train_filename_format,
            X_test_filename_format,
            y_test_filename_format,
            predict_threshold):

        row = idx.copy()
        row["accuracy_train"] = accuracy_score(y, y_hat)
        row["precision_train"] = precision_score(y, y_hat)
        row["recall_train"] = recall_score(y, y_hat)

        row["accuracy_test"] = accuracy_score(y_test, y_test_hat)
        row["precision_test"] = precision_score(y_test, y_test_hat)
        row["recall_test"] = recall_score(y_test, y_test_hat)

        rows.append(row)

    performance_metrics = pd.DataFrame(rows)
    performance_metrics["yq"] = (performance_metrics["yq"] + 1) / 10

    performance_metrics.to_csv(os.path.join(target_folder, "performance_metrics.csv"))


if __name__ == "__main__":
    process_pdl1_experiment_2_cross("PDL1_experiment_2_cross")
