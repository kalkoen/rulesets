import json

import pandas as pd
from results import *

from tex import *


def interpretable_results_PDL1_experiment_2(experiment_name,
                                            result_file_pattern,
                                            X_filename_format,
                                            y_filename_format,
                                            gen_subfolder,
                                            tables=None):
    data_folder = f"data/{experiment_name}"
    results_folder = f"data/{experiment_name}/results"

    columns = {
        "yq": int,
        "mw": float,
        "C": int,
        "D": int,
    }

    if tables is None:
        tables = {
            "tab_interpretable_mw_0_C_1": [
                {
                    "yq": 4,
                    "mw": 0.0,
                    "C": 1,
                    "D": 1
                },
                {
                    "yq": 4,
                    "mw": 0.0,
                    "C": 1,
                    "D": 2
                },
                {
                    "yq": 4,
                    "mw": 0.0,
                    "C": 1,
                    "D": 3
                },
                {
                    "yq": 5,
                    "mw": 0.0,
                    "C": 1,
                    "D": 1
                },
                {
                    "yq": 5,
                    "mw": 0.0,
                    "C": 1,
                    "D": 2
                },
                {
                    "yq": 5,
                    "mw": 0.0,
                    "C": 1,
                    "D": 3
                }],
            "tab_interpretable_mw_0_C_2": [
                {
                    "yq": 4,
                    "mw": 0.0,
                    "C": 2,
                    "D": 1
                },
                {
                    "yq": 4,
                    "mw": 0.0,
                    "C": 2,
                    "D": 2
                },
                {
                    "yq": 4,
                    "mw": 0.0,
                    "C": 2,
                    "D": 3
                },
                {
                    "yq": 5,
                    "mw": 0.0,
                    "C": 2,
                    "D": 1
                },
                {
                    "yq": 5,
                    "mw": 0.0,
                    "C": 2,
                    "D": 2
                },
                {
                    "yq": 5,
                    "mw": 0.0,
                    "C": 2,
                    "D": 3
                }
            ]
        }

    for table, idxs in tables.items():
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
            }
        )

        pd.set_option("display.max_columns", None)
        df_interpretable = df_interpretable.drop("mw", axis=1)
        df_interpretable["yq"] = (df_interpretable["yq"] + 1) / 10.0
        df_interpretable["yq"] = df_interpretable["yq"].map(lambda x: f"{x:.1f}" if isinstance(x, float) else x)
        df_interpretable.columns = ["yq", "C", "D", "Acc.", "Prec.", "Rec.", "Ruleset"]
        df_interpretable = df_interpretable.set_index(["yq", "C", "D"])
        df_interpretable.columns = [r"\textbf{" + col + "}" for col in df_interpretable.columns]
        df_interpretable.index.names = [r"$\mathbf{Q_y}$", r"$\mathbf{C}$", r"$\mathbf{D}$"]

        df_interpretable.to_latex(
            tex_gen_file(f"{gen_subfolder}/{gen_subfolder}_{table}.tex"),
            float_format="%.2f",
            # index=False
        )


if __name__ == "__main__":
    interpretable_results_PDL1_experiment_2(
        "PDL1_experiment_2",
        result_file_pattern=r"yq_{yq}_mw_{mw}_C_{C}_settings_D_{D}.set.json",
        X_filename_format="X.csv",
        y_filename_format="y_q_{yq}.csv",
        gen_subfolder="PDL1ex2")
