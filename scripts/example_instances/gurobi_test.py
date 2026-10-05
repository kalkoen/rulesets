from collections import defaultdict
import numpy as np
import pandas as pd
import pyomo.environ as pyo

m = pyo.ConcreteModel()

X_file = "data/PDL1_experiment_2/X.csv"
y_file = "data/PDL1_experiment_2/y_q_6.csv"
X = pd.read_csv(X_file)
y = pd.read_csv(y_file)
y = y[y.columns[0]]

n_quantiles = 5
quantiles = np.linspace(1/(n_quantiles+1), 1-1/(n_quantiles+1), n_quantiles)
margin_width = 0.6

print("quantiles", quantiles)

n_datapoints = len(y)
assert len(X) == len(y)

mu = np.zeros(n_datapoints)
mu[y == 1] = 1.0

clauses = []
for feature in X.columns:
    for quantile in X[feature].quantile(quantiles):
        clauses.append((feature, quantile))

P = list(np.where(y == 1)[0])
N = list(np.where(y == 0)[0])

m.P = pyo.Set(initialize=P)
m.N = pyo.Set(initialize=N)
m.L = pyo.Set(initialize=clauses)

def gamma_datapoint_clause(i, l, beta):
    (feat, thres) = l
    if y.loc[i] == 1:
        return max(0, 1 - 1/beta * max(0, thres - X[feat].loc[i]))
    else:
        return min(1, 1/beta * max(0, X[feat].loc[i] - thres))

# Precompute gammas
gamma = {}
for i in N:
    for l in clauses:
        gamma[i, l] = gamma_datapoint_clause(i, l, margin_width)

# Partition clauses per datapoint
trivial_zero = defaultdict(list)
trivial_one  = defaultdict(list)
nontrivial   = defaultdict(list)

eps = 1e-9
for i in N:
    for l in clauses:
        g = gamma[i, l]
        if g < eps:
            trivial_zero[i].append(l)
        elif g > 1 - eps:
            trivial_one[i].append(l)
        else:
            nontrivial[i].append(l)

nontrivial_pairs = [(i, l) for i in N for l in nontrivial[i]]

m.Gp = pyo.Var(m.P, domain=pyo.Reals)
m.x  = pyo.Var(nontrivial_pairs, domain=pyo.NonNegativeReals, bounds=(0, 1))
m.x0 = pyo.Var(N, domain=pyo.NonNegativeReals, bounds=(0, 1))
m.x1 = pyo.Var(N, domain=pyo.NonNegativeReals, bounds=(0, 1))
m.z  = pyo.Var(m.L, domain=pyo.Binary)

def pos_cons(m, i, feat, thres):
    return m.Gp[i] <= 1 - (1 - gamma_datapoint_clause(i, (feat, thres), margin_width)) * m.z[(feat, thres)]
m.pos_cons = pyo.Constraint(m.P, m.L, rule=pos_cons)

def neg_cons(m, i, feat, thres):
    l = (feat, thres)
    if l in nontrivial[i]:
        return m.x[i, l] <= m.z[l]
    return pyo.Constraint.Skip
m.neg_conss = pyo.Constraint(N, clauses, rule=neg_cons)

def neg_cons_zero(m, i):
    if not trivial_zero[i]:
        return pyo.Constraint.Skip
    return m.x0[i] <= sum(m.z[l] for l in trivial_zero[i])
m.neg_cons_zero = pyo.Constraint(N, rule=neg_cons_zero)

def neg_cons_one(m, i):
    if not trivial_one[i]:
        return pyo.Constraint.Skip
    return m.x1[i] <= sum(m.z[l] for l in trivial_one[i])
m.neg_cons_one = pyo.Constraint(N, rule=neg_cons_one)

def eq_cons(m, i):
    nt_sum = sum(m.x[i, l] for l in nontrivial[i]) if nontrivial[i] else 0
    return m.x0[i] + m.x1[i] + nt_sum == 1
m.eq_conss = pyo.Constraint(N, rule=eq_cons)

clauses_by_feature = defaultdict(list)
for (feat, thres) in clauses:
    clauses_by_feature[feat].append((feat, thres))

def one_clause_per_feature(m, feat):
    return sum(m.z[l] for l in clauses_by_feature[feat]) <= 1
m.one_clause = pyo.Constraint(list(clauses_by_feature.keys()), rule=one_clause_per_feature)

def obj(m):
    neg_obj = (
        sum(m.x1[i] for i in N) +
        sum(gamma[i, l] * m.x[i, l] for (i, l) in nontrivial_pairs)
    )
    pos_obj = sum(-mu[i] * m.Gp[i] for i in m.P)
    return pos_obj + neg_obj

def max_clauses_cons(m):
    return sum(m.z[l] for l in m.L) <= 3
m.max_clauses = pyo.Constraint(rule=max_clauses_cons)

m.obj = pyo.Objective(rule=obj, sense=pyo.minimize)

solver = pyo.SolverFactory('highs')
# solver.options['presolving/maxrounds'] = -1   # unlimited presolve rounds
# solver.options['presolving/maxrestarts'] = -1
# solver.options['separating/maxroundsroot'] = -1  # unlimited cut rounds at root
# solver.options['separating/aggressive'] = True
# solver.options['numerics/feastol'] = 1e-9
# solver.options['emphasis/numerics'] = 0  # default numerics
# solver.options['lp/initalgorithm'] = 'b'  # barrier for root LP, helps big-M
# solver.options['separating/clique/freq'] = 1
# solver.options['separating/clique/maxrounds'] = -1
# solver.options['separating/clique/maxroundsroot'] = -1
# solver.options['heuristics/rens/freq'] = 1
# solver.options['heuristics/rins/freq'] = 1
# solver.options['heuristics/feaspump/freq'] = 1

# solver.options['SCIP_emphasis'] = 'optimality'
# solver.config.stream_solver = True
solver.highs_options = {'output_flag': True, 'log_to_console': True}
results = solver.solve(m, tee=True)

print(results)

print("z (selected clauses):")
for l in m.L:
    if pyo.value(m.z[l]) > 0.5:
        print(f"  {l}")

print("\nGp (positive point scores):")
for i in m.P:
    print(f"  i={i}: Gp={pyo.value(m.Gp[i]):.4f}")

print("\nObjective:", pyo.value(m.obj))