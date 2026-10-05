import pandas as pd
from sklearn.metrics import accuracy_score

from py.model import parse_string_ruleset, classify_ruleset, Clause
from py.greedy import solve_greedy
from tex import format_ruleset_latex

experiments = [
    {
        "C": 1,
        "Qy": 0.5,
        "D": 1,
    },
    {
        "C": 1,
        "Qy": 0.5,
        "D": 2,
    },
    {
        "C": 1,
        "Qy": 0.5,
        "D": 3,
    },
    {
        "C": 1,
        "Qy": 0.6,
        "D": 1,
    },
    {
        "C": 1,
        "Qy": 0.6,
        "D": 2,
    },
    {
        "C": 1,
        "Qy": 0.6,
        "D": 3,
    },
]


X_sanger = pd.read_csv("data/X_TFactivities_Sanger.csv").set_index("sample")
X_sanger = X_sanger.pivot(columns="gene", values="TF activity score")
y_sanger = pd.read_csv("data/Y_PDL1expression_Sanger.csv").set_index("sample")["PDL1 expression"]

rows = []

for experiment in experiments:
    row = experiment.copy()
    print(row)

    y_threshold = y_sanger.quantile(experiment["Qy"])
    y_sanger_target = (y_sanger >= y_threshold).astype(int)

    ruleset, df = solve_greedy(X_sanger, y_sanger_target, row["C"], row["D"], 0)
    row["ruleset"] = df.loc[0, "rule"]
    row["accuracy"] = df.loc[0, "accuracy"]

    rows.append(row)

df = pd.DataFrame(rows)
df.to_csv("data/PDL1_experiment_5/optimal.csv")