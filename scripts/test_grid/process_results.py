import json

import pandas as pd

from scripts.gridplot import plot_2d_grid, ruleset_to_regions

with open("data/test_grid/results_uncorrected/O_mw_0.00001.json") as f:
    result = json.load(f)

X = pd.read_csv("data/test_grid/X_1.csv")
y = pd.read_csv("data/test_grid/y_1.csv")["y"]

regions = ruleset_to_regions(result["binary"])

plot_2d_grid(
    X,
    y,
    (X["x1"].min(), X["x1"].max()),
    (X["x2"].min(), X["x2"].max()),
    regions = regions,
    n_gridlines=(9,9),
    title="Grid test"
)