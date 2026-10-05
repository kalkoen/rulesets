import json

import pandas as pd
from results import *

from tex import *

experiment_name = "PDL1_experiment_2_cross"
result_file_pattern = r"yq_{yq}_s_{s}_mw_{mw}_C_{C}_D_{D}.json"
X_filename_format = "y_q_{yq}_s_{s}_train_X.csv"
y_filename_format = "y_q_{yq}_s_{s}_train_y.csv"

data_folder = f"data/{experiment_name}/split"
results_folder = f"data/{experiment_name}/results"

columns = {
    "yq": int,
    "mw": float,
    "C": int,
    "D": int,
    "s": int
}

splits = range(10)
tables = {
    f"tab_cross_interpretable_mw_{mw}_C_1": [
        {
            "yq": 4,
            "mw": mw,
            "C": 1,
            "D": 3
        },
        {
            "yq": 4,
            "mw": mw,
            "C": 1,
            "D": 2
        },
        {
            "yq": 5,
            "mw": mw,
            "C": 1,
            "D": 3
        },
        {
            "yq": 5,
            "mw": mw,
            "C": 1,
            "D": 2
        }
    ] for mw in [0, 0.2, 0.4, 0.6, 0.8, 1.0]
}

def feature_formatter(ruleset, X):
    return r" $\vee$ ".join([
        r"{" +
        r" $\wedge$ ".join([clause.split(">=")[0] for clause in rule]) +
        r"}"
    for rule in sorted(ruleset)])

for table, idxs in tables.items():

    idxs = [
        idx | {"s": s} for s in splits for idx in idxs
    ]

    df_interpretable = get_interpretable_result_df(
        iterator_X_y_pred(
            get_results_from_folder(
                results_folder,
                result_file_pattern,
                columns,
                idxs
            ),
            data_folder,
            X_filename_format,
            y_filename_format,
            predict_threshold
        ),
        performance_metric_dict={
            "accuracy": accuracy_score,
            "precision": precision_score,
            "recall": recall_score
        },
        ruleset_formatter=feature_formatter
    )

    pd.set_option("display.max_columns", None)
    df_interpretable = df_interpretable.drop("mw", axis=1)
    df_interpretable["yq"] = (df_interpretable["yq"] + 1) / 10.0
    df_interpretable["yq"] = df_interpretable["yq"].map(lambda x: f"{x:.1f}" if isinstance(x, float) else x)
    df_counts = df_interpretable.groupby(["yq", "C", "D"])["ruleset"].value_counts().rename("Frequency").to_frame()
    df_counts.columns = [r"\textbf{" + col + "}" for col in df_counts.columns]
    df_counts.index.names = [r"$\mathbf{Q_y}$", r"$\mathbf{C}$", r"$\mathbf{D}$", r"\textbf{TF combination}"]

    df_counts.to_latex(
        tex_gen_file(f"PDL1ex2cross/{table}.tex"),
        float_format="%.2f",
        # index=False
    )