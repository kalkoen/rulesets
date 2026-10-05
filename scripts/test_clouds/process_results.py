import json
import os
from itertools import chain, combinations

import pandas as pd
from matplotlib import pyplot as plt

import re
from scripts.gridplot import plot_2d_grid, ruleset_to_regions, _fmt_sci
from scripts.tex import tex_gen_file
from scripts.verification import compute_objective


def nonempty_subsets(iterable):
    s = list(iterable)
    return chain.from_iterable(combinations(s, r) for r in range(1, len(s)))

def log(*args, file, sep=" ", end="\n"):
    print(*args, sep=sep, end=end)
    print(*args, sep=sep, end=end, file=file)


results_folders = ["results"]  # or results_uncorrected
folder = "data/test_clouds/"

model_type_map = {
    "additive": "Additive",
    "nonadditive": "Non-additive"
}

model_type_huge_plots

with open(os.path.join(folder, "proces_results.out", ), "w") as output:

    for results_folder in results_folders:
        folder_results = os.path.join(folder, results_folder)

        with open(os.path.join(folder, "examples.json"), "r") as f:
            examples = json.load(f)

            for fname in sorted(os.listdir(folder_results)):
                if not fname.endswith(".json"):
                    continue

                result_file_pattern = r"^O_(\d+)_C_(\d+)_mw_(\d+(?:\.\d+)?)_(\w+)\.set\.json$"
                idx, C, mw, model_type = re.match(result_file_pattern, fname).groups()
                mw = float(mw)

                log("==Investigating ", fname, " idx=", idx, " C=", C, " mw=", mw, "type=", model_type, file=output)
                output.flush()

                example = examples[idx]
                X = pd.read_csv(os.path.join(folder, f"X_{idx}.csv"))
                y = pd.read_csv(os.path.join(folder, f"y_{idx}.csv"))["y"]

                with open(os.path.join(folder_results, fname)) as f:
                    result = json.load(f)

                    regions = ruleset_to_regions(result["binary"])

                    plot_2d_grid(
                        X,
                        y,
                        (X[X.columns[0]].min(), X[X.columns[0]].max()),
                        (X[X.columns[1]].min(), X[X.columns[1]].max()),
                        regions=regions,
                        n_gridlines=(9, 9),
                        # suptitle=example['title'],
                        title=f"{model_type_map[model_type]} model, "
                              f"$C = {result['C']}$, "
                              f"$\\beta={_fmt_sci(result['margin_width'])}$",
                        margin_width=result['margin_width'],
                        # top=1
                    )

                    fig_name = fname.replace(".json", ".svg")
                    plt.savefig(tex_gen_file(f"test_clouds/{results_folder}/{fig_name}"))
                    plt.show()

                    ruleset = result["binary"]

                    obj_full = compute_objective(ruleset, X, y, mw)

                    log("Full set ", ruleset, " has objective ", obj_full, " reported ", result["obj"], file=output)
                    output.flush()

                    for subset in [[["x2>=9.88"]]] + list(nonempty_subsets(ruleset)):

                        obj_sub = compute_objective(subset, X, y, mw)

                        suffix = "larger."
                        if  abs(obj_sub - obj_full) < 1e-5:
                            suffix = "EQUAL!"
                        elif obj_sub < obj_full:
                            suffix = "SMALLER???"

                        log("Subset ", subset, " has objective ", obj_sub, ". Is", suffix, file=output)
                        output.flush()
