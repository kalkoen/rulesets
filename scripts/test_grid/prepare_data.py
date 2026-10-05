import os

import numpy as np
import pandas as pd

import matplotlib.pyplot as plt
from scripts.gridplot import plot_2d_grid

folder_out = 'data/test_grid'
margin = 0.5

n = 10

x1, x2 = np.mgrid[0:n, 0:n]
y = np.zeros((n,n))
y[6:, 7:] = 1
y[:2, 4:] = 1
y[3:, :1] = 1

X = pd.DataFrame({"x1": x1.ravel(), "x2": x2.ravel()})
y = pd.Series(y.flatten(), name="y").astype(int)

demo_regions = [
    # [("x1", 5, ">="), ("x2", 6, ">=")],
    # [("x1", 3, "<=")],
    # [("x2", 4, "<="), ("x1", 8, ">=")],
]

plot_2d_grid(
    X,
    y,
    (-margin, n-1 + margin),
    (-margin, n-1 + margin),
    demo_regions,
    n_gridlines=(n-2, n-2)
)
plt.show()

os.makedirs(folder_out, exist_ok=True)
X.to_csv(os.path.join(folder_out, 'X_1.csv'), index=False)
y.astype(int).to_csv(os.path.join(folder_out, 'y_1.csv'), index=False)

print("# datapoints:", y)
print("of which positive:", y.sum())