import os

import pandas as pd
import seaborn as sns
from matplotlib import pyplot as plt

from tex import tex_gen_file

performance_metrics = pd.read_csv("data/PDL1_experiment_2_cross/results/performance_metrics.csv")
print(performance_metrics)

id_cols = [c for c in performance_metrics.columns
           if c not in (
                    "accuracy_train", "accuracy_test",
                        # "precision_train", "precision_test",
                        # "recall_train", "recall_test"
                        )
           ]
long_df = performance_metrics.melt(
    id_vars=id_cols,
    value_vars=["accuracy_train", "accuracy_test",
                "precision_train", "precision_test",
                "recall_train", "recall_test"],
    var_name="metric_split", value_name="value"
)

g = sns.relplot(
    data=long_df[long_df["yq"]!=0.7],
    kind="line",
    marker="o",
    y="value",
    x="C",
    hue="metric_split",
    row="yq", col="mw"
)


g.set_titles(row_template="$Q_y = {row_name}$", col_template=r"$\beta = {col_name}$")

g.set(xticks=sorted(long_df["C"].unique()))

g.figure.suptitle(r"Train and test accuracy for $D=2$ and various C, $\beta$", fontsize=20, y=.98)
g.figure.subplots_adjust(top=0.9)
plt.savefig(
    tex_gen_file("PDL1ex2cross/train_test_mw.svg"))
plt.show()
