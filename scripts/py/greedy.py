
from collections import defaultdict
from enum import Enum
from sklearn.metrics import accuracy_score
from tqdm import tqdm

from scripts.py.model import *
from scripts.py.model import Clause
from scripts.tex import format_rule_latex, tex_gen_file

class NegateType(Enum):
    OFF = 0
    NO_MIXING = 1
    MIXING = 2

def feature_clauses(X: npt.NDArray[np.float32], q_list: Iterable[np.float32]):
    clauses = []
    for i in range(X.shape[1]):
        for q in q_list:
            q_val = np.quantile(X[:, i], q)
            clauses.append(NpClause(feature=i, threshold=q_val))
    return clauses

def feature_np_clauses(X: npt.NDArray[np.float32], q_list: Iterable[np.float32]):
    clauses = []
    for i in range(X.shape[1]):
        for q in q_list:
            q_val = np.quantile(X[:, i], q)
            clauses.append(NpClause(feature=i, threshold=q_val))
    return clauses

def generator_np_rules_recurse(clauses: List[NpClause],
                            clauses_per_feature: Mapping[int, Iterable[NpClause]],
                            D: int,
                            base_rule: List[NpClause],
                            features_to_consider: Set[int],
                            forbidden_feature_combinations: Mapping[int, Iterable[int]]):
    if len(base_rule) == D:
        return
    else:
        for feature in features_to_consider:
            features_to_consider.remove(feature)
            for forbidden_feature in forbidden_feature_combinations[feature]:
                features_to_consider.remove(forbidden_feature)

            for clause in clauses_per_feature[feature]:
                new_rule = base_rule + [clause]
                yield new_rule
                yield from generator_np_rules_recurse(clauses,
                                                   clauses_per_feature,
                                                   D,
                                                   new_rule,
                                                   features_to_consider,
                                                   forbidden_feature_combinations)

            features_to_consider.add(feature)
            for forbidden_feature in forbidden_feature_combinations[feature]:
                features_to_consider.add(forbidden_feature)


def generator_np_rules(clauses: List[NpClause],
                    clauses_per_feature: Mapping[int, Iterable[NpClause]],
                    D: int,
                    forbidden_feature_combinations: Mapping[int, Iterable[int]]):
    yield from generator_np_rules_recurse(clauses,
                                       clauses_per_feature,
                                       D,
                                       [],
                                       set(clauses_per_feature.keys()),
                                       forbidden_feature_combinations)


def find_rule(X: pd.DataFrame,
              y: pd.Series,
              D: int,
              beta: float=0.0,
              n_quantiles: int = 9,
              q_list: List[np.float32] | None = None,
              negate: NegateType = NegateType.NO_MIXING):
    assert X.index.equals(y.index)
    assert set(y.unique()) == {0, 1}
    X = X.sort_index()
    y = y.sort_index()

    if negate != NegateType.OFF:
        cols = X.columns
        neg_X = -X
        neg_X.columns = ["-" + col for col in cols]
        X = pd.concat([X, neg_X], axis=1)

    feature_names: List[str] = list(X.columns)
    X_np = X.to_numpy()
    y_np = y.to_numpy()

    if q_list is None:
        q_list = np.linspace(0, 1, n_quantiles + 2)[1:-1]

    clauses = feature_np_clauses(X_np, q_list)
    # for clause in clauses:
    #     print(clause)

    clauses_per_feature = defaultdict(list)
    for clause in clauses:
        clauses_per_feature[clause.feature].append(clause)

    forbidden_feature_combinations = defaultdict(list)

    total_rules = 0
    for rule_size in range(1, D + 1):
        total_rules += len(q_list) ** rule_size

    min_obj = np.finfo(np.float32).max
    best_rule = []
    for rule in tqdm(generator_np_rules(clauses,
                                     clauses_per_feature,
                                     D,
                                     forbidden_feature_combinations), total=total_rules, ncols=80):
        obj = objective_np_rule(X_np, y_np, beta, rule)
        if obj < min_obj:
            best_rule = rule
            min_obj = obj

    string_rule = [f"{feature_names[clause.feature]}>={clause.threshold}" for clause in best_rule]
    print(string_rule)
    rule = parse_string_ruleset([string_rule], X)

    return rule[0], min_obj


def solve_greedy(X: pd.DataFrame,
                 y: pd.Series,
                 C: int,
                 D: int,
                 beta: float = 0.0,
                 n_quantiles: int = 9,
                 q_list: List[np.float32] | None = None,
                 negate: NegateType = NegateType.NO_MIXING,
                 verbose=True):
    """Greedily builds a ruleset that classifies a binary target using threshold rules.

    Iteratively finds the rule (conjunction of up to D threshold-feature clauses)
    that best improves the classification objective, adds it to the ruleset, and
    removes the positive points it now correctly catches before the next iteration.
    Stops early once no further improvement is possible or after C iterations.

    Args:
        X: Continuous input data. Index must match y's index.
        y: Binary target data ({0, 1}). Index must match X's index.
        C: Maximum number of iterations (rules) to generate.
        D: Maximum number of threshold-feature clauses (TF) per rule. Note that
            D=3 is may already be slow.
        beta: Margin width for the ramp loss model. When greater than 0, the
            ramp loss (instead of the standard 0/1 loss) is used with this
            margin. Defaults to 0.0 (no margin).
        n_quantiles: Number of quantile thresholds to consider per TF when
            beta is 0 and q_list is not given. Defaults to 9.
        q_list: Optional explicit list of quantile thresholds to consider,
            overriding n_quantiles. Defaults to None.
        negate: Strategy for negating clauses within a rule. Defaults to
            NegateType.NO_MIXING.
        verbose: Whether to print detailed logs during solving. Defaults to True.

    Returns:
        A tuple (ruleset, history):
            ruleset: List of rules, each rule a list of Clause objects.
            history: DataFrame with one row per iteration, including the
                iteration number, dataset size, the rule found, how many
                points it newly classifies, and cumulative accuracy.
    """

    assert X.index.equals(y.index)
    assert set(y.unique()) == {0, 1}
    assert beta >= 0.0

    ruleset: List[List[Clause]] = []
    records = []

    y_it = y.copy()
    X_it = X.copy()


    for it in range(C):
        record = {}
        record["it"] = it
        record["num_total"] = len(y_it)
        record["num_pos"] = y_it.sum()
        record["num_neg"] = (1-y_it).sum()

        base_obj = y_it.sum()*2.0

        if verbose:
            print(f"--- Starting iteration {it}.", base_obj)
            print(f"Intermediate dataset has {record["num_total"] } points ({record["num_pos"]} pos, {record["num_neg"]} neg)")
            print(f"Base objective {(base_obj):.2f}")

        new_rule, best_obj = find_rule(X_it, y_it, D, beta, n_quantiles, q_list, negate)

        if best_obj > base_obj:
            print("No increase in objective possible. Breaking.")
            break

        ruleset.append(new_rule)
        y_after = classify_rule(X_it, new_rule)

        record["rule"] =  new_rule
        record["num_caught"] = (y_after + y_it == 2).sum()
        record["num_pos_classified"] = y_after.sum()
        record["frac_of_total"] = y_after.sum()/len(y)


        if verbose:
            print(f"Newly found rule: ", new_rule)
            print(f"Positively classifies {y_after.sum()} more points (fraction {record["frac_of_total"]:.2f} of total).")
            print(f"This is {record["num_caught"]} positive points of total {record["num_pos"]} positive points.")

        y_it = y_it[y_after == 0].copy()
        X_it = X_it[y_after == 0].copy()

        y_hat = classify_ruleset(X, ruleset)

        record["num_pos_left"] = y_it.sum()
        record["accuracy"] = accuracy_score(y, y_hat)

        if verbose:
            print(f"Objective after: {best_obj}")
            print("Total accuracy up until now: ", record["accuracy"])

        records.append(record)

    print("~ Finished.")
    print("Final objective:", objective_ruleset(X, y, beta, ruleset))
    y_hat = classify_ruleset(X, ruleset)
    print("Final accuracy:", accuracy_score(y, y_hat))

    return ruleset, pd.DataFrame(records)

def save_greedy_result_latex(ruleset, df, file):
    cols = ["it", "num_total", "num_pos", "num_pos_classified", "num_caught", "accuracy", "rule"]
    df["it"] = df["it"] + 1
    df[cols].to_latex(
        file,
        header=[r"$\bm{{\tau}}$",
                r"\textbf{{\makecell{{\#\\ in $\I_\tau$}}}}",
                r"\textbf{{\makecell{{\#\\ in $\P_\tau$}}}}",
                r"\textbf{{\makecell{{\#\\covered\\ by $k_\tau$}}}}",
                r"\textbf{{\makecell{{\# in $\P_\tau$ \\ covered \\ by $k_\tau$}}}}",
                r"\textbf{{\makecell{{Acc.}}}}",
                r"\textbf{{Rule}}"],
        formatters={
            "rule": format_rule_latex,
        },
        float_format="%.2f",
        index=False)


# if __name__ == "__main__":
#     X = pd.read_csv("data/PDL1_experiment_2/X.csv").astype(np.float32)
#     y = pd.read_csv("data/PDL1_experiment_2/y_q_5.csv")
#     y = y[y.columns[0]].astype(np.int32)
#     ruleset, df = solve_greedy(X, y, C=2, D=2, beta=0.0, n_quantiles=9)
#     print(ruleset)
#     print(df)
