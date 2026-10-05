import ast

import pandas as pd

from py.model import parse_string_ruleset
from tex import tex_gen_file, format_rule_latex

df = pd.read_csv("data/PDL1_experiment_5/optimal.csv").drop("Unnamed: 0", axis=1)
df["Qy"] = df["Qy"].map("{:.1f}".format)
df = df.set_index(["Qy", "C", "D"])
print(df["ruleset"])
df["ruleset"] = df["ruleset"].apply(lambda s: [x.strip() for x in s.strip("[]").split(",")])
df = df[["accuracy","ruleset"]]
df.index.names = [
    r"$\mathbf{{Q_y}}$",
    r"$\mathbf{{C}}$",
    r"$\mathbf{{D}}$"]
df.to_latex(
    tex_gen_file("holdout/tab_optimal_C_1.tex"),
    index_names=True,
    formatters={
        "ruleset": format_rule_latex
    },
    header=[
        r"\textbf{{Acc.}}",
        r"\textbf{{Ruleset}}",
    ],
    float_format="%.2f",
)