import numpy as np
import pandas as pd

from py.greedy import solve_greedy, save_greedy_result_latex
from tex import format_rule_latex, tex_gen_file

experiments = [
    # {
    #     "yq": 4,
    #     "mw": 0,
    #     "C": 2,
    #     "D": 3
    # },
    {
        "yq": 4,
        "mw": 0,
        "C": 3,
        "D": 2
    },
    # {
    #     "yq": 5,
    #     "mw": 0,
    #     "C": 2,
    #     "D": 3
    # },
    {
        "yq": 5,
        "mw": 0,
        "C": 3,
        "D": 2
    },
    {
        "yq": 4,
        "mw": 0,
        "C": 3,
        "D": 1
    },
    {
        "yq": 5,
        "mw": 0,
        "C": 3,
        "D": 1
    }
]

for idx in experiments:
    print("> Experiment", idx)
    X = pd.read_csv("data/PDL1_experiment_2/X.csv").astype(np.float32)
    y = pd.read_csv(f"data/PDL1_experiment_2/y_q_{idx['yq']}.csv")
    y = y[y.columns[0]].astype(np.int32)
    ruleset, df = solve_greedy(X, y, C=idx["C"], D=idx["D"], beta=idx["mw"], n_quantiles=9)

    save_greedy_result_latex(
        ruleset,
        df,
        tex_gen_file("PDL1ex2/tab_greedy_yq_{yq}_mw_{mw}_C_{C}_D_{D}.tex".format(**idx)))