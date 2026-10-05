import os

import numpy as np
import pandas as pd
import seaborn as sns
from matplotlib import pyplot as plt
from numpy.ma.core import shape

from tex import tex_gen_file


def plot_results_PDL1_experiment_2(experiment_name,
                                   gen_subfolder):
    target_folder = f"data/{experiment_name}/results"
    performance_metrics = pd.read_csv(os.path.join(target_folder, "performance_metrics.csv"))

    cmap = sns.color_palette("rocket", as_cmap=True)
    g = sns.relplot(
        data=performance_metrics[performance_metrics["mw"] == 0],
        x="C", y="accuracy",
        hue="yq",
        style="yq",
        row="D",
        kind="line",
        marker="o",
        height=3, aspect=1.5,
        facet_kws={"margin_titles": True},
        palette=[cmap(x) for x in np.linspace(0.1, 0.75, 5)]
    )
    g.legend.set_title("$Q_y$")
    g.set(ylim=(0.5, 1))
    g.set(ylabel="Accuracy")
    g.set_titles(row_template="D={row_name}")
    g.figure.suptitle("Performance by $Q_y$", y=1)
    g.set(xticks=performance_metrics["C"].unique())
    sns.move_legend(g, "upper left", bbox_to_anchor=(1, 1))
    g.figure.subplots_adjust(right=0.9, top=0.95)

    # for row_val, ax_row in zip(g.row_names, g.axes):
    #     for ax in ax_row:
    #         ax.axhline(row_val, ls=":", color="gray")

    plt.savefig(tex_gen_file(f"{gen_subfolder}/{gen_subfolder}_fig_accuracy_mw_0.svg"),
                bbox_inches="tight")
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    plot_results_PDL1_experiment_2("PDL1_experiment_2", "PDL1ex2")
