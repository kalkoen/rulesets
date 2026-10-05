import json
import os

import numpy as np
import pandas as pd
from matplotlib import pyplot as plt

from scripts.gridplot import plot_2d_grid
from scripts.tex import tex_gen_file

random_state = 42
np.random.seed(random_state)

folder = 'data/test_clouds'
margin = 0.5
n_quantiles = 9

with open(os.path.join(folder, "examples.json"), "r") as f:
    examples = json.load(f)

    for idx, example in examples.items():
        Xs = []
        ys = []
        clouds = example["clouds"]
        for cloud in clouds:
            points = np.random.multivariate_normal(
                mean=cloud["center"],
                cov=np.eye(2) * cloud["std"]**2,
                size=cloud["n"],
            )
            Xs.append(pd.DataFrame({"x1": points[:, 0], "x2": points[:, 1]}))
            ys.append(pd.Series([cloud["y"]] * cloud["n"]))
        X = pd.concat(Xs, ignore_index=True)
        y = pd.concat(ys, ignore_index=True)

        centers = [cloud["center"] for cloud in clouds]

        if example["x1_lim"] is not None:
            example["x1_lim"] = np.add(example["x1_lim"], (-margin, margin))
        if example["x2_lim"] is not None:
            example["x2_lim"] = np.add(example["x2_lim"], (-margin, margin))

        plot_2d_grid(
            X,
            y,
            example["x1_lim"],
            example["x2_lim"],
            None,
            n_gridlines=(n_quantiles, n_quantiles),
            cloud_centers=centers,
            title=example["title"],
            feature_labels = ("$x_1$", "$x_2$"),
        )

        plt.savefig(tex_gen_file(f"test_clouds/example_{idx}.svg"))
        plt.show()

        os.makedirs(folder, exist_ok=True)
        X.to_csv(os.path.join(folder, f'X_{idx}.csv'), index=False)
        y.astype(int).to_frame("y").to_csv(os.path.join(folder, f'y_{idx}.csv'), index=False)

        print("# datapoints:", len(y))
        print("of which positive:", y.sum())
