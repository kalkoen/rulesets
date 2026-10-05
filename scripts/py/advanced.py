import json
import os
from typing import List

import numpy as np
import pandas as pd
import uuid
import shutil

from py.model import parse_string_ruleset
from scripts.py.greedy import NegateType


import subprocess

def solve_advanced(X: pd.DataFrame,
                   y: pd.Series,
                   C: int,
                   D: int,
                   beta: float = 0.0,
                   n_quantiles: int = 9,
                   q_list: List[np.float32] | None = None,
                   negate: NegateType = NegateType.NO_MIXING,
                   exact_enabled: bool = False,
                   verbose=True,
                   temp_folder: str = "temp/",
                   settings_file: str = None,
                   cg_timelimit: int = 0,
                   timelimit: int = 0,
                   executable: str | None = "build/rs"):
    """Learns a binary-classification ruleset by calling an external solver.

    Writes X, y and a solver settings file to ``temp_folder``, runs
    ``executable`` as a subprocess, reads its JSON output, deletes the
    temporary files and parses the result into a ruleset. Each rule is a
    conjunction of up to D threshold-feature clauses (TFs).

    Args:
        X: Continuous input data. Index must match y's index (both are
            sorted by index before export).
        y: Binary target data ({0, 1}).
        C: Maximum number of rules. Passed to the solver as a CLI argument.
        D: Maximum number of TFs per rule.
        beta: Margin width for the ramp loss. If > 0, the ramp loss is used
            instead of the 0/1 loss. Passed as a CLI argument. Defaults to 0.0.
        n_quantiles: Number of quantile thresholds per feature. Ignored if
            q_list is given. Defaults to 9.
        q_list: Explicit quantile thresholds, overriding n_quantiles.
        negate: Clause negation strategy. Defaults to NegateType.NO_MIXING.
        exact_enabled: Whether to enable the exact (non-heuristic) pricing.
            Defaults to False.
        verbose: Currently unused.
        temp_folder: Directory for temporary I/O files. Defaults to "temp/".
        settings_file: Path to a solver settings file. If given, it is used
            as-is and D, n_quantiles, q_list, negate, exact_enabled,
            cg_timelimit and timelimit are ignored. C and beta are still
            passed via the CLI.
        cg_timelimit: Column-generation time limit in seconds. 0 means no
            limit. Defaults to 0.
        timelimit: Overall solver time limit in seconds. 0 means no limit.
            Defaults to 0.
        executable: Path to the solver binary. Defaults to "build/rs".

    Returns:
        A tuple (ruleset, data):
            ruleset: List of rules, each a list of Clause objects, parsed
                from ``data["binary"]``.
            data: The raw JSON output of the solver as a dict.

    Raises:
        FileNotFoundError: If the solver fails to produce the output file.
    """

    ident = uuid.uuid4()

    os.makedirs(temp_folder, exist_ok=True)

    X_filename = os.path.join(temp_folder,f"{ident}_X.csv")
    y_filename = os.path.join(temp_folder,f"{ident}_y.csv")
    O_filename = os.path.join(temp_folder,f"{ident}_O.json")

    S_filename = os.path.join(temp_folder, f"{ident}_S.set")
    if settings_file is None:
        print("Using settings passed as parameters")
        with open(S_filename, "w") as f:

            f.write("rs/type = 1 \n")
            f.write("rs/method = 1 \n")
            f.write("rs/rule_complexity_type = 0 \n")

            f.write(f"rs/threshold/negation = {str(negate.value)} \n")

            if q_list is None:
                f.write(f"rs/threshold/quantiles = \"{n_quantiles}\" \n")
            else:
                f.write(f"rs/threshold/quantiles = \"{','.join([str(q) for q in q_list])}\" \n")

            f.write("rs/threshold/nonadditive = true \n")

            f.write(f"rs/exact/enabled = {str(exact_enabled).lower()} \n")
            f.write(f"rs/exact/D = {D} \n")

            f.write("rs/heuristic/enabled=True \n")
            f.write(f"rs/heuristic/beam_widths = \"{','.join(["1000"]*D)}\" \n")
            f.write("rs/heuristic/deep = false \n")
            f.write("rs/heuristic/max_candidates = 10 \n")
            f.write("rs/heuristic/diversity = 1 \n")

            if cg_timelimit > 0:
                f.write(f"rs/cg_timelimit = {cg_timelimit} \n")
            if timelimit > 0:
                f.write(f"limits/time = {timelimit} \n")
    else:
        print("Using settings file", settings_file)
        shutil.copy2(settings_file, S_filename)

    X.sort_index().to_csv(X_filename, index=False)
    y.astype(int).sort_index().to_csv(y_filename, index=False)

    args = [str(executable), str(X_filename), str(y_filename), str(C), str(O_filename), str(S_filename), str(beta)]

    if verbose:
        result = subprocess.run(
            args  # path to exe + any CLI arg
        )
    else:
        result = subprocess.run(
            args,
            stdout=None,
            stderr=None,
            capture_output=True
        )

    d = None
    with open(O_filename) as f:
        d = json.load(f)

    os.remove(X_filename)
    os.remove(y_filename)
    os.remove(O_filename)
    os.remove(S_filename)

    ruleset = parse_string_ruleset(d["binary"], X)

    return ruleset, d
