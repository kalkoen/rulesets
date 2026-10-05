import pandas as pd
from sklearn.metrics import accuracy_score

from py.model import parse_string_ruleset, classify_ruleset, Clause
from tex import format_ruleset_latex, format_bare_ruleset_latex, tex_gen_file

experiments = [
    {
        "C": 1,
        "Qy": 0.5,
        "D": 1,
        "ruleset": [["STAT2>=-0.20"]]
    },
    {
        "C": 1,
        "Qy": 0.5,
        "D": 2,
        "ruleset": [["STAT2>=-0.20", "FOS>=-1.26"]],
        # Do something with RELA and STAT2
    },
    {
        "C": 1,
        "Qy": 0.5,
        "D": 3,
        "ruleset": [["STAT2>=-0.20", "FOS>=-1.26", "IRF1>=-1.59"]]
    },
    {
        "C": 1,
        "Qy": 0.6,
        "D": 1,
        "ruleset": [["FOSL1>=0.41"]]
    },
    {
        "C": 1,
        "Qy": 0.6,
        "D": 2,
        "ruleset": [["FOSL1>=0.13", "STAT2>=-0.74"]],
        # Do something with RELA and STAT2
    },
    {
        "C": 1,
        "Qy": 0.6,
        "D": 3,
        "ruleset": [["FOSL1>=0.13", "STAT2>=-0.74", "FOS>=-0.78"]]
    },
]

X_broad = pd.read_csv("data/X_TFactivities_Broad.csv").set_index("sample")
X_broad = X_broad.pivot(columns="gene", values="TF activity score")
y_broad = pd.read_csv("data/Y_PDL1expression_Broad.csv").set_index("sample")["PDL1 expression"]
X_sanger = pd.read_csv("data/X_TFactivities_Sanger.csv").set_index("sample")
X_sanger = X_sanger.pivot(columns="gene", values="TF activity score")
y_sanger = pd.read_csv("data/Y_PDL1expression_Sanger.csv").set_index("sample")["PDL1 expression"]

rows = []

for experiment in experiments:
    row = experiment.copy()
    del row["ruleset"]

    print(experiment)
    rp = parse_string_ruleset(experiment["ruleset"], X_broad)
    print(rp)

    y_threshold = y_broad.quantile(experiment["Qy"])

    y_broad_target = (y_broad >= y_threshold)
    y_hat_broad = classify_ruleset(X_broad, rp)
    row["accuracy_broad"] = accuracy_score(y_broad_target, y_hat_broad)

    y_sanger_target_absolute = (y_sanger >= y_threshold)
    y_hat_sanger_absolute = classify_ruleset(X_sanger, rp)
    row["accuracy_sanger_absolute"] = accuracy_score(y_sanger_target_absolute, y_hat_sanger_absolute)

    y_threshold_sanger_relative = y_sanger.quantile(experiment["Qy"])
    y_sanger_target_relative = (y_sanger >= y_threshold_sanger_relative)
    rp_sanger_relative = [[Clause(c.feature, X_sanger[c.feature].quantile(c.q), c.q, c.lb) for c in rule] for rule in
                          rp]
    y_hat_sanger_relative = classify_ruleset(X_sanger, rp_sanger_relative)
    row["accuracy_sanger_relative"] = accuracy_score(y_sanger_target_relative, y_hat_sanger_relative)

    row["ruleset"] = format_bare_ruleset_latex(experiment["ruleset"], X_broad)

    rows.append(row)

df = pd.DataFrame(rows)
df["Qy"] = df["Qy"].map("{:.1f}".format)
df = df.set_index(["Qy", "C", "D"])
df.index.names = [
    r"$\mathbf{{Q_y}}$",
    r"$\mathbf{{C}}$",
    r"$\mathbf{{D}}$"]
df.to_latex(
    tex_gen_file("holdout/tab_C_1.tex"),
    index_names=True,
    header=[
        r"\textbf{{\makecell{{Acc.\\Broad}}}}",
        r"\textbf{{\makecell{{Acc.\\Sanger\\(Abs.)}}}}",
        r"\textbf{{\makecell{{Acc.\\Sanger\\(Rel.)}}}}",
        r"\textbf{{Ruleset}}"
    ],
    float_format="%.2f",
)
