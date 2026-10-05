import numpy as np
import pandas as pd
import seaborn as sns
from matplotlib import pyplot as plt

from results import *
from results import predict_threshold

from tex import *

def process_pdl1_experiment_2(experiment_name,
                              result_file_pattern=
                              r"yq_{yq}_mw_{mw}_C_{C}_settings_D_{D}.set.json",
                              X_filename_format="X.csv",
                              y_filename_format="y_q_{yq}.csv"):

    data_folder = f"data/{experiment_name}"
    target_folder = f"data/{experiment_name}/results"

    columns = {
        "yq": int,
        "mw": float,
        "C": int,
        "D": int
    }

    results = get_results_from_folder(target_folder, result_file_pattern, columns)
    # results_df = get_results_df(results, columns)

    results_X_y_pred = iterator_X_y_pred(
        results,
        data_folder,
        X_filename_format,
        y_filename_format,
        predict_threshold)

    # X_sanger = pd.read_csv(os.path.join(data_folder, "sanger","X.csv"))

    rows = []
    for idx, result, X, y, y_hat in results_X_y_pred:
        row = idx.copy()
        row["accuracy"] = accuracy_score(y, y_hat)
        row["precision"] = precision_score(y, y_hat)
        row["recall"] = recall_score(y, y_hat)
        #
        # y_sanger = pd.read_csv(os.path.join(data_folder, "sanger",f"y_q_{idx['yq']}.csv"))
        # y_hat_sanger = predict_threshold(result["binary"], X_sanger)
        #
        # row["accuracy_sanger"] = accuracy_score(y_sanger, y_hat_sanger)
        # row["precision_sanger"] = precision_score(y_sanger, y_hat_sanger)
        # row["recall_sanger"] = recall_score(y_sanger, y_hat_sanger)

        rows.append(row)

    performance_metrics = pd.DataFrame(rows)
    performance_metrics["yq"] = (performance_metrics["yq"] + 1)/10

    performance_metrics.to_csv(os.path.join(target_folder, "performance_metrics.csv"))

    g = sns.relplot(
        data=performance_metrics,
        x="mw", y="accuracy",
        hue="D",
        row="yq", col="C",
        kind="line",
        marker="o",
        height=3, aspect=1.2,
        facet_kws={"margin_titles": True},
    )
    g.set(ylim=(0, 1))
    g.set_titles(row_template="yq={row_name}", col_template="C={col_name}")

    for row_val, ax_row in zip(g.row_names, g.axes):
        for ax in ax_row:
            ax.axhline(row_val, ls=":", color="gray")

    plt.show()

    pmt = performance_metrics[(performance_metrics["mw"] == 0) & (performance_metrics["D"] == 3)]
    # for metric in ["accuracy", "precision", "recall", "accuracy_sanger", "precision_sanger", "recall_sanger"]:
    for metric in ["accuracy", "precision", "recall"]:
        print("D=3", metric,":")
        print(pmt.pivot(
            index = "yq",
            columns = "C",
            values= metric
        ))

    pmt = performance_metrics[(performance_metrics["mw"] == 0) & (performance_metrics["D"] == 2)]
    # for metric in ["accuracy", "precision", "recall", "accuracy_sanger", "precision_sanger", "recall_sanger"]:
    for metric in ["accuracy", "precision", "recall"]:
        print("D=2", metric,":")
        print(pmt.pivot(
            index = "yq",
            columns = "C",
            values= metric
        ))


if __name__ == "__main__":
    process_pdl1_experiment_2("PDL1_experiment_2")