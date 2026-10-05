import os

import numpy as np
import pandas as pd
import seaborn as sns
from matplotlib import pyplot as plt
from numpy.ma.core import shape

from tex import tex_gen_file


def plot_results_PDL1_experiment_1(experiment_name, gen_subfolder):
    results_folder = f"data/{experiment_name}/results"
    performance_metrics = pd.read_csv(os.path.join(results_folder, "performance_metrics.csv"))

    cmap = sns.color_palette("rocket", as_cmap=True)
    g = sns.relplot(
        data=performance_metrics[performance_metrics["yq"] >= 0.5],
        x="Xnq", y="accuracy",
        hue="yq",
        style="yq",
        kind="line",
        marker="o",
        height=3, aspect=1.5,
        facet_kws={"margin_titles": True},
        palette=[cmap(x) for x in np.linspace(0.1, 0.75, 5)]
    )

    g.legend.set_title("$Q_y$")
    g.set(ylim=(0.5, 1))
    g.set(ylabel="Accuracy")
    g.set(xlabel=r"$n_{\text{quantiles}}$")

    g.ax.set_title("Performance by $Q_y$", y=1)

    g.set(xticks=performance_metrics["Xnq"].unique())
    sns.move_legend(g, "upper left", bbox_to_anchor=(1, 1))
    g.figure.subplots_adjust(right=0.95, top=0.95)

    # for row_val, ax_row in zip(g.row_names, g.axes):
    #     for ax in ax_row:
    #         ax.axhline(row_val, ls=":", color="gray")

    plt.savefig(tex_gen_file(f"{gen_subfolder}/fig_{gen_subfolder}_accuracy.svg"),
                bbox_inches="tight")
    plt.tight_layout()
    plt.show()

    records = pd.read_csv(os.path.join(results_folder, "records.csv"))

    time_limit = "Limit"

    converged = records[["Xnq","yq","obj", "obj_cg", "obj_relaxation", "status_cg"]].copy()
    converged = converged[converged["yq"].astype(int) >= 4]
    converged["gap"] = (converged["obj"] - converged["obj_cg"])/converged["obj_cg"]
    converged["gap"] = converged["gap"].astype("object")
    converged.loc[converged["status_cg"] != 1, "gap"] = time_limit
    converged = converged.pivot(columns="Xnq", index="yq", values="gap")
    converged.index.name = r"\textbf{$Q_y$}"
    converged.columns.name = r"\textbf{$n_{\text{quantiles}}$}"
    converged.index = ((converged.index + 1) / 10)

    print(converged)

    def color_cell(val):
        color = "C6E0B4" if val != time_limit else "F4CCCC"
        return f"background-color: #{color}"

    def fmt_num(val):
        if isinstance(val, str):
            return val  # e.g. "Time limit" stays as-is
        return f"\\num{{{val:.3f}}}"

    styled = (
        converged.style
        .map(color_cell)
        .format(fmt_num)
        .format_index(lambda v: f"\\textbf{{{v}}}", axis=0)
        .format_index(lambda v: f"\\textbf{{{v}}}", axis=1)
    )

    latex_code = styled.to_latex(
        tex_gen_file(f"{gen_subfolder}/tab_{gen_subfolder}_gap.tex"),
        convert_css=True,
        hrules=True,
        column_format='l' + ( 'r' * (len(converged.columns)+1))
    )
    print(styled.to_latex(
        convert_css=True,
        hrules=True,
        column_format='l' + ( 'r' * (len(converged.columns)+1))
    ))


if __name__ == "__main__":
    plot_results_PDL1_experiment_1(
        "PDL1_experiment_1",
        "PDL1ex1")
