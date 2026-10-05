from collections import defaultdict

import pandas as pd
import numpy as np


def compute_objective(ruleset, X, y, mw):
    compound_gammas = pd.Series(0.0, index=y.index)
    for k in ruleset:
        compound_gammas += gamma_k(X, y, k, mw)

    pos_mask = (y == 1)

    result = compound_gammas.copy()
    result[pos_mask] = np.maximum(0, 2.0 - compound_gammas[pos_mask])
    return result.sum()


def gamma_k(X, y, k, mw):
    return pd.concat([gamma_ja(X, y, clause, mw) for clause in k], axis=1).min(axis=1)


def gamma_ja(X, y, clause, mw):
    feature, threshold = clause.split(">=")
    threshold = float(threshold)

    x_j = -X[feature[1:]] if feature.startswith("-") else X[feature]

    if mw == 0:
        return (x_j >= threshold).astype(int) * 2.0
    else:
        return np.clip(1 + 1/mw * (x_j - threshold), 0.0, 2.0)
