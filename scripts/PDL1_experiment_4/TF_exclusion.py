import pandas as pd

from py.greedy import solve_greedy, save_greedy_result_latex

X_TF_activities_broad = pd.read_csv("data/X_TFactivities_Broad.csv")
X = X_TF_activities_broad.pivot(values="TF activity score", index="sample", columns=["gene"])
y = pd.read_csv("data/Y_PDL1expression_Broad.csv", index_col="sample")["PDL1 expression"]

print(X)

Qy = 0.5  # PD-L1 expression quantile (PD-L1 expression >= Q(Qy))
D = 2  # max number of clauses per rule
mw = 0  # Zero means the threshold model rather than the ramp loss model

accuracy_cutoff = 0.6

print(f"Running exclusion experiment on Broad dataset with Qy={Qy} D={D} mw={mw}, accuracy cutoff={accuracy_cutoff}")

y_q = (y >= y.quantile(Qy)).astype(int)
TF = X.columns.unique()
print("TF:", TF)

ruleset_baseline, df_baseline = solve_greedy(X, y_q, C=1, D=D, beta=mw, n_quantiles=9, verbose=False)

rulesets = {frozenset({"baseline"}): ruleset_baseline}
dfs = {frozenset({"baseline"}): df_baseline}
print("Ruleset:", ruleset_baseline)

exclusion_combos = [frozenset({clause.feature}) for clause in ruleset_baseline[0]]

i = 0

while i < len(exclusion_combos):
    exclude_TFs = exclusion_combos[i]
    print("~ Excluding", exclude_TFs)
    ruleset, df = solve_greedy(X.drop(columns=exclude_TFs),
                               y_q,
                               C=1,
                               D=D,
                               beta=mw,
                               n_quantiles=9,
                               verbose=False)
    rule = ruleset[0]
    rulesets[exclude_TFs], dfs[exclude_TFs] = rule, df
    print("Ruleset:", rule)

    if df.loc[0, "accuracy"] > accuracy_cutoff and len(exc):
        for clause in rule:
            new_exclusion_combo = exclude_TFs | {clause.feature}
            if new_exclusion_combo not in exclusion_combos:
                exclusion_combos.append(new_exclusion_combo)

    print()

    i+=1

master_df = pd.concat([v.assign(source=str(k)) for k, v in dfs.items()], ignore_index=True)
pd.set_option("display.max_columns", None)
