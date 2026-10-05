import itertools
import os
import numpy as np
import pandas as pd
import seaborn as sns
from matplotlib import pyplot as plt
from tqdm import tqdm
from tex import tex_gen_file


def plot_results_PDL1_experiment_2_cross(experiment_name):
    target_folder = f"data/{experiment_name}/results"
    performance_metrics = pd.read_csv(os.path.join(target_folder, "performance_metrics.csv"))
    id_cols = [c for c in performance_metrics.columns
               if c not in ("accuracy_train", "accuracy_test",
                            "precision_train", "precision_test",
                            "recall_train", "recall_test")]
    long_df = performance_metrics.melt(
        id_vars=id_cols,
        value_vars=["accuracy_train", "accuracy_test",
                    "precision_train", "precision_test",
                    "recall_train", "recall_test"],
        var_name="metric_split", value_name="value"
    )
    long_df[["metric", "split"]] = long_df["metric_split"].str.rsplit("_", n=1, expand=True)
    sub = long_df[(long_df["mw"] == 0) & (long_df["D"] == 2)
                  & (long_df["yq"].isin([0.5, 0.6]))
                  & (long_df["metric"] == "accuracy")]

    sub["split"] = sub["split"].str.capitalize()

    yq_values = sorted(sub["yq"].unique())
    fig, axes = plt.subplots(len(yq_values), 1, figsize=(5, 7), sharex=True)

    for ax, yq in zip(axes, yq_values):
        data = sub[sub["yq"] == yq]
        sns.lineplot(
            data=data,
            x="C", y="value",
            hue="split",
            marker="o",
            palette={"Train": "#4C72B0", "Test": "#DD8452"},
            ax=ax,
            legend=(ax is axes[0]),  # only show legend once
        )
        ax.set_ylim(0.5, 1)
        ax.set_xticks(sorted(data["C"].unique()))
        ax.set_ylabel("Accuracy")
        ax.set_title(f"$Q_y = {yq}$")

    axes[-1].set_xlabel("$C$")
    handles, labels = axes[0].get_legend_handles_labels()
    axes[0].legend_.remove()
    fig.legend(handles, labels, title="Split", bbox_to_anchor=(0.95, 0.9), loc="upper left")

    fig.suptitle("Train and test performance by $Q_y$ for $D=2$")
    plt.savefig(
        tex_gen_file("PDL1ex2/fig_train_test_accuracy_mw_0.svg"),
        bbox_inches="tight")
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    plot_results_PDL1_experiment_2_cross("PDL1_experiment_2_cross")