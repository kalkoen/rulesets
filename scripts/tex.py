import os
from pathlib import Path

import pandas as pd

from scripts.results import format_bare_ruleset

GEN_FOLDER = "tex/gen/"

def tex_gen_file(filename, root=None):
    if root is not None:
        p = Path(root)
        p = os.path.join(p, GEN_FOLDER)
    else:
        p = Path(GEN_FOLDER)
    p = Path(os.path.join(p, filename))
    p.parent.mkdir(parents=True, exist_ok=True)
    return p

def aligned_value_parentheses_table(df, filename, caption=None, label=None, max_col_width='1.7cm', exclude_columns=None):
    if exclude_columns is None:
        exclude_columns = []

    # Function to split the string and wrap in LaTeX
    def format_latex_num(val):
        # Split by space
        parts = val.split()
        if len(parts) == 2:
            return rf"\num{{{parts[0]}}} & (\num{{{parts[1]}}})"
        return val  # Return original if it doesn't match the pattern

    df = df.astype(str).map(format_latex_num)

    headers = [r'\parbox{{' + max_col_width + r'}}{{ \textbf{{ ' + df.index.name + '}}}}']
    column_format = "l "
    for col in df.columns:
        if col in exclude_columns:
            headers.append(r"\textbf{{" + col + "}}")
            column_format += r' V{' + max_col_width + r'}'
        else:
            column_format += ' r@{ }l'
            headers.append(r"\multicolumn{{2}}{{c}}{{\parbox{{" + str(max_col_width) + r"}}{{\centering \textbf{{" + str(col) + "}}}}}}")

    df.reset_index().to_latex(
        tex_gen_file(filename),
        index=False,
        escape=False,
        column_format=column_format,
        caption=caption,
        label=label,
        # CRITICAL: Double the braces {{ }} for multicolumn
        # to prevent the "return value.format(x)" error.
        header=headers,
    )

def format_rule_latex(rule):
    return format_ruleset_latex([rule])

def format_ruleset_latex(ruleset):
    return (r"\makecell{" +
            r"\\ $\vee$ \\".join(
        [r"\{" + r" $\wedge$ \\".join(
            [r"$\text{" +
             str(clause).replace(" <=",r"} \leq").replace(" >=",r"} \geq").replace("Q", r"\text{ Q}")
             + "$" for clause in rule]
        ) + r"\}" for rule in ruleset]) +
            "}"
        )

def format_bare_ruleset_latex(ruleset, X):
    ruleset_with_q = format_bare_ruleset(ruleset, X)
    return format_ruleset_latex(ruleset_with_q)


def get_interpretable_result_df(results_X_y_pred,
                                performance_metric_dict = None,
                                ruleset_formatter=format_bare_ruleset_latex):
    if performance_metric_dict is None:
        performance_metric_dict = {}
    records = []
    for idx, result, X, y, pred in results_X_y_pred:
        d = idx.copy()
        for metric_name, metric_func in performance_metric_dict.items():
            d[metric_name] = metric_func(y, pred)
        d["ruleset"] = ruleset_formatter(result["binary"], X)
        records.append(d)
    return pd.DataFrame(records)