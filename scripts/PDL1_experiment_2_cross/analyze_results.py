import itertools

import numpy as np
import pandas as pd
import seaborn as sns
from matplotlib import pyplot as plt
from tqdm import tqdm

from results import *


def analyze_pdl1_experiment_2_cross(experiment_name,
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
    for idx, result, X, y in iterator_X_y(results, data_folder, X_train_filename_format, y_train_filename_format):
        if idx["mw"] == 0 and idx["C"] == 1 and idx["yq"] == 5 and idx["D"] == 2:
            print(idx)
            print(format_bare_ruleset(result["binary"], X))
            print()


if __name__ == "__main__":
    analyze_pdl1_experiment_2_cross("PDL1_experiment_2_cross")
