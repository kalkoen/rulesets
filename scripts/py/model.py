
from typing import List, Mapping, Set
from typing import Iterable
from dataclasses import dataclass

import numpy as np
import numpy.typing as npt
import pandas as pd
from scipy.stats import percentileofscore


@dataclass
class NpClause:
    feature: int
    threshold: float


@dataclass
class Clause:
    feature: str
    threshold: float
    q: float | None
    lb: bool

    def __repr__(self) -> str:
        direction = ">=" if self.lb else "<="
        if self.q is not None:
            return f"{self.feature} {direction} {self.threshold:.3f} Q({self.q:.3f})"
        else:
            return f"{self.feature} {direction} {self.threshold:.3f}"


def G_np_rule(X_np: npt.NDArray[np.float32], beta: float, np_rule: List[NpClause]):
    thresholds = np.array([clause.threshold for clause in np_rule])
    features = np.array([clause.feature for clause in np_rule])
    if beta == 0:
        return (X_np[:, features] >= thresholds).astype(np.float32) * 2
    else:
        return np.clip(1 + 1 / beta * (X_np[:, features] - thresholds), 0, 2)

def g_np_rule(X: npt.NDArray[np.float32], beta, rule: List[NpClause]):
    return np.min(G_np_rule(X, beta, rule), axis=1)

def objective_np_rule(X: npt.NDArray[np.float32], y: npt.NDArray[np.int32], beta, rule: List[NpClause]) -> float:
    g = g_np_rule(X, beta, rule)
    obj_pos = np.sum(2 - g[y == 1])
    obj_neg = np.sum(g[y == 0])
    return obj_pos + obj_neg

def objective_np_ruleset(X: npt.NDArray[np.float32], y: npt.NDArray[np.int32], beta, ruleset: List[List[NpClause]]) -> float:
    assert len(ruleset) > 0
    g_rules = np.column_stack([g_np_rule(X, beta, rule) for rule in ruleset])
    obj_pos = np.sum(2 - np.max(g_rules[y == 1, :], axis=1))
    obj_neg = np.sum(g_rules[y==0, :])
    return obj_pos + obj_neg

def objective_ruleset(X: pd.DataFrame, y: pd.Series, beta: float, ruleset: List[List[Clause]]) -> float:
    assert X.index.equals(y.index)
    X_neg = -X
    X_neg.columns = ["-" + col for col in X.columns]
    X = pd.concat([X, X_neg],axis=1)
    X_np = X.sort_index().to_numpy()
    y_np = y.sort_index().to_numpy()
    col_map = {val: idx for idx, val in enumerate(X.columns)}
    np_ruleset = [[NpClause(
        feature=col_map[clause.feature if clause.lb else "-" + clause.feature],
        threshold=clause.threshold if clause.lb else -clause.threshold) for clause in rule] for rule in ruleset]
    return objective_np_ruleset(X_np, y_np, beta, np_ruleset)

def classify_ruleset(X: pd.DataFrame, ruleset: List[List[Clause]]):
    y = pd.Series(False, index=X.index)
    for rule in ruleset:
        y |= classify_rule(X, rule)
    return y

def classify_rule(X: pd.DataFrame, rule: List[Clause]):
    y = pd.Series(True, index=X.index)
    for clause in rule:
        if clause.lb:
            y &= X[clause.feature] >= clause.threshold
        else:
            y &= X[clause.feature] <= clause.threshold
    return y

def parse_string_ruleset(string_ruleset: List[List[str]], X: pd.DataFrame | None = None):
    def parse_string_clause(string_clause):
        feat, val = string_clause.split(">=")
        val = float(val)
        if string_clause.startswith("-"):
            feat = feat[1:]
            val = -val
            return Clause(feature=feat,
                          threshold=val,
                          lb=False,
                          q=percentileofscore(X[feat], float(val), kind='mean')/100,) if X is not None else None
        else:
            return Clause(feature=feat,
                          threshold=val,
                          lb=True,
                          q=percentileofscore(X[feat], float(val), kind='mean')/100,) if X is not None else None

    return [[parse_string_clause(string_clause) for string_clause in string_rule] for string_rule in string_ruleset]