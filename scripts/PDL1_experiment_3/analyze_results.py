import json

from results import *

from tex import *
import seaborn as sns
import matplotlib.pyplot as plt

experiment_name = "PDL1_experiment_3"
result_file_pattern = r"yq_{yq}_mw_{mw}_C_{C}_settings_D_{D}.set.json"
X_filename_format = "X_broad.csv"
y_filename_format = "y/y_broad_q_{yq}.csv"

data_folder = f"data/{experiment_name}"
target_folder = f"data/{experiment_name}/results"

idx1 = {
    "yq": 5,
    "mw": 0,
    "C": 1,
    "D": 2
}
result1 = get_result(target_folder, result_file_pattern, idx1)
ruleset = result1["binary"]
X1, y1 = load_X_y(idx1, data_folder, X_filename_format, y_filename_format)

ruleset_formatted = [[format_bare_clause(clause, X1) for clause in rule] for rule in result1["binary"]]

print(json.dumps(ruleset_formatted, indent=2))
print(accuracy_score(y1, predict_threshold(ruleset, X1)))

ruleset_test = [['FOSL1>=109.350000', '-NFATC1>=-3.920000']]

print(accuracy_score(y1, predict_threshold(ruleset_test, X1)))

performance_metrics = pd.read_csv(os.path.join(target_folder, "performance_metrics.csv"))

pmt = performance_metrics[
    (performance_metrics["mw"] == 0)
    & (performance_metrics["yq"] <= 0.6)
    & (performance_metrics["D"] == 2)
    ]
ax = sns.lineplot(
    data=pmt,
    x="C",
    y="accuracy",
    hue="yq",
    marker="o",
)
ax.set_xticks(sorted(pmt["C"].unique()))
ax.set_ylim(0, 1)
ax.legend(title="PDL1 expression quantile")
plt.show()
