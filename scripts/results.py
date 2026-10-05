import json
import os
import re

import numpy as np
import pandas as pd
from sklearn.metrics import accuracy_score, precision_score, recall_score, confusion_matrix, f1_score, fbeta_score, \
    matthews_corrcoef
import numbers
from parse import parse

def predict_binary(ruleset, X):
    if not ruleset:
        return pd.Series(False, index=X.index)

    rule_results = []
    for rule in ruleset:
        rule_satisfied = X[rule].all(axis=1)
        rule_results.append(rule_satisfied)

    return pd.concat(rule_results, axis=1).any(axis=1)

def predict_threshold(ruleset, X):
    if not ruleset:
        return pd.Series(False, index=X.index)

    result = pd.Series(False, index=X.index)
    for rule in ruleset:
        rule_satisfied = pd.Series(True, index=X.index)
        for clause in rule:
            rule_satisfied &= clause_name_to_mask(clause, X)
        result |= rule_satisfied
    return result

def clause_name_to_mask(clause_name, X, offset=0.0):
    feature, value = clause_name.split(">=")
    value = float(value)
    if feature.startswith("-"):
        return -X[feature[1:]] >= value + offset
    else:
        return X[feature] >= value + offset

def get_result(folder_path, pattern, idx):
    with open(os.path.join(folder_path, pattern.format(**idx)), "r") as f:
        return json.load(f)

def get_results_from_folder(folder_path, pattern, columns, idxs = None):
    results = []
    for filename in os.listdir(folder_path):
        parsed = parse(pattern, filename)
        if parsed is not None:
            identifier_dict = parsed.named
            for k, v in identifier_dict.items():
                identifier_dict[k] = columns[k](v)
            if idxs is None or identifier_dict in idxs:
                file_path = os.path.join(folder_path, filename)
                try:
                    with open(file_path, 'r') as f:
                        results.append((identifier_dict, json.load(f)))
                except (json.JSONDecodeError, OSError) as e:
                    print(f"Skipping {filename} due to error: {e}")

    if idxs is not None:
        results.sort(key=lambda r: idxs.index(r[0]))

    return results


def get_results_df(results, columns):
    rows = []
    column_types = columns.copy()
    for idx, result in results:
        d = {}
        for key, val in result.items():
            if isinstance(val, str) or isinstance(val, numbers.Number):
                d[key] = val
                column_types[key] = type(val)

        for col_name, var in idx.items():
            d[col_name] = var
        rows.append(d)
    return pd.DataFrame(rows).astype(column_types)


def load_X_y(idx, data_folder, filename_X_format, filename_y_format):
    return (pd.read_csv(os.path.join(data_folder, filename_X_format.format(**idx))),
            pd.read_csv(os.path.join(data_folder, filename_y_format.format(**idx))))

def iterator_X_y(results, data_folder, filename_X_format, filename_y_format):
    for idx, result in results:
        X, y = load_X_y(idx, data_folder, filename_X_format, filename_y_format)
        yield idx, result, X, y

def iterator_X_y_pred(results,
                      data_folder,
                      filename_X_format,
                      filename_y_format,
                      pred_func):
    for idx, result in results:
            X, y = load_X_y(idx, data_folder, filename_X_format, filename_y_format)
            y_hat = pred_func(result["binary"],X)
            yield idx, result, X, y, y_hat

def iterator_X_y_pred_train_test(results, data_folder,
                                 filename_X_train_format,
                                 filename_y_train_format,
                                 filename_X_test_format,
                                 filename_y_test_format,
                                 pred_func):
    for idx, result, X, y, y_hat in iterator_X_y_pred(results, data_folder, filename_X_train_format, filename_y_train_format, pred_func):
        X_test, y_test = load_X_y(idx, data_folder, filename_X_test_format, filename_y_test_format)
        y_test_hat = pred_func(result["binary"],X_test)
        yield idx, result, X, y, y_hat, X_test, y_test, y_test_hat



def get_mean_std_table(results_df, group, group_name=None, metric=None, metric_name=None, decimals=1):
    if isinstance(group, str):
        group = [group]
    if isinstance(group_name, str) and group_name is not None:
        group_name = [group_name]

    if metric is not None:
        results_df = results_df[[*group, metric]]
        if metric_name is not None:
            results_df = results_df.rename({metric: metric_name}, axis=1)
    mean = (results_df.groupby(group).mean() * 100).map(lambda x: f"{x:.{decimals}f}")
    std = (results_df.groupby(group).std() * 100).map(lambda x: f"{x:.{decimals}f}")
    table = mean + " " + std
    if group_name is not None:
        table.index.names = group_name
    return table


def get_pricing_steps_df(results_dict, col_names):
    rows = []
    for vars, result in results_dict.items():
        for step in result["pricing_steps"]:
            for col_name, var in zip(col_names, vars):
                step[col_name] = var
            rows.append(step)
    return pd.DataFrame(rows).astype("object").infer_objects()


def get_computed_metrics(results_X_y,
                    columns,
                    computed_metrics_dict) -> pd.DataFrame:
    rows = []
    col_types = columns.copy()
    for idx, result, X, y in results_X_y.items():
        if "binary" not in result:
            print("Warning", columns.keys(), idx, "has no ruleset as result. Probably ran out of time.")
            continue

        d = {}
        for col_name, val in idx.items():
            d[col_name] = val
        for name, metric_func in computed_metrics_dict.items():
            d[name] = metric_func(result)
            col_types[name] = type(d[name])
        rows.append(d)
    return pd.DataFrame(rows).astype(col_types)


def get_performance_metrics(results_X_y,
                            columns,
                            performance_metric_dict,
                            predict_func) -> pd.DataFrame:
    rows = []
    col_types = columns.copy()
    for idx, result, X, y in results_X_y:

        if len(X) == 0:
            print("Warning: an X in data folder is empty, with indices", idx)
            continue

        if "binary" not in result:
            print("Warning", columns.keys(), idx, "has no ruleset as result. Probably ran out of time.")
            continue

        y_hat = predict_func(result["binary"], X)

        d = {}
        for col_name, val  in idx.items():
            d[col_name] = val
        for name, metric_func in performance_metric_dict.items():
            d[name] = metric_func(y, y_hat)
            col_types[name] = type(d[name])
        rows.append(d)
    return pd.DataFrame(rows).astype(col_types)


def default_performance_metrics_binary():
    return {
        f"accuracy": accuracy_score,
        f"precision": precision_score,
        f"recall": recall_score,
        f"f5": lambda y, y_hat: fbeta_score(y, y_hat, beta=5),
        "mcc": matthews_corrcoef,
    }

def default_computed_metrics():
    return {
        f"complexity": lambda r: len(r["binary"]) + sum(len(rule) for rule in r["binary"]),
        f"number of rules": lambda r: len(r["binary"]),
        f"average rule complexity": lambda r:
        sum(len(rule) for rule in r["binary"]) / len(r["binary"]) if len(r["binary"]) > 0 else 0
    }

def get_default_metrics_binary(results_dict, results_df, results_X_y, columns) -> pd.DataFrame:

    mapping = {
        'obj': 'objective',
        'obj_cg': 'relaxation objective',
        "status_cg": 'status_cg',
        'soltime_cg': 'solve time'
    }

    recorded_metrics = results_df.set_index(list(columns))[list(mapping.keys())].rename(columns=mapping)

    performance_metrics = get_performance_metrics(results_X_y,
                                                  columns,
                                                  default_performance_metrics_binary(),
                                                  predict_binary)

    computed_metrics = get_computed_metrics(results_dict,
                                            columns,
                                            default_computed_metrics())

    metrics_df = pd.concat([
                            recorded_metrics,
                            performance_metrics.set_index(list(columns)),
                            computed_metrics.set_index(list(columns))],
                           axis=1)

    return metrics_df

def format_bare_clause(clause, X):
    feat,val = clause.split(">=")
    operator = ">="
    if feat.startswith("-"):
        feat = feat[1:]
        val = -float(val)
        operator = "<="
    q = inverse_quantile(X[feat], val)
    return f"{feat} {operator} {float(val):.2f} Q({q:.2f})"

def format_bare_ruleset(ruleset, X):
    return [[format_bare_clause(clause, X) for clause in rule] for rule in ruleset]

def inverse_quantile(series, value):
    s = np.sort(series.values)
    return np.interp(value, s, np.linspace(0, 1.0, len(s)))