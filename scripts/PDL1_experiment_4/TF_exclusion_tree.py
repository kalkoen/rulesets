import pandas as pd

from PDL1_experiment_4.tree_to_table import Node, to_latex
from py.greedy import solve_greedy, save_greedy_result_latex
from tex import format_rule_latex, tex_gen_file

# X_TF_activities_broad = pd.read_csv("data/X_TFactivities_Broad.csv")
# X = X_TF_activities_broad.pivot(values="TF activity score", index="sample", columns=["gene"])
# y = pd.read_csv("data/Y_PDL1expression_Broad.csv", index_col="sample")["PDL1 expression"]

X = pd.read_csv("data/PDL1_experiment_3/X_broad_with_sample.csv").set_index("sample")
y = pd.read_csv("data/PDL1_experiment_3/y_broad_with_sample.csv").set_index("sample")["CD274"]
suffix = "_tpm"

print(X)

Qy = 0.5  # PD-L1 expression quantile (PD-L1 expression >= Q(Qy))
D = 1  # max number of clauses per rule
mw = 0  # Zero means the threshold model rather than the ramp loss model

depth = 3
include_depth_col = False

print(f"Running exclusion experiment on Broad dataset with Qy={Qy} D={D} mw={mw}, depth={depth}")

y_q = (y >= y.quantile(Qy)).astype(int)
TF = X.columns.unique()
print("TF:", TF)

ruleset_baseline, df_baseline = solve_greedy(X, y_q, C=1, D=D, beta=mw, n_quantiles=9, verbose=False)

root = Node(
    children = [],
    value = ("Baseline: ", f"{df_baseline['accuracy'].loc[0]:.2f}"),
    data = (ruleset_baseline[0], df_baseline, []),
    score = df_baseline["accuracy"].loc[0]
)
layers = [[root]]

max_accuracy = root.score
min_accuracy = root.score

while len(layers) <= depth:

    new_layer = []

    print("Building layer", len(layers) + 1)

    for node in layers[len(layers) - 1]:

        print("Exploring node", node.value)

        node_rule, node_df, excluded = node.data
        for clause in node_rule:
            TF = clause.feature
            print("> Excluding", TF)
            exclude_TFs = excluded + [TF]
            ruleset, df = solve_greedy(X.drop(columns=exclude_TFs),
                                       y_q,
                                       C=1,
                                       D=D,
                                       beta=mw,
                                       n_quantiles=9,
                                       verbose=False)
            rule = ruleset[0]

            new_node = Node(
                children = [],
                value = (f"-{TF}:", f"{df['accuracy'].loc[0]:.2f}"),
                score = df["accuracy"].loc[0],
                data=(rule,df,exclude_TFs)
            )
            min_accuracy = min(min_accuracy, new_node.score)
            node.children.append(new_node)
            new_layer.append(new_node)

    layers.append(new_layer)

    print()

latex = to_latex(root, min_accuracy, max_accuracy, depth_col=include_depth_col)
print(latex)
with open(tex_gen_file(f"exclusion/tab_exclusion_Qy_{Qy}_D_{D}_mw_{mw}_depth_{depth}{suffix}.tex"), "w") as f:
    f.write(latex)
