from pyscipopt import Model

drop_c_d = False
vartype_z =  "CONTINUOUS" # BINARY OR CONTINUOUS

# ------------------------------------------------------------------ #
# Instance data                                                        #
# ------------------------------------------------------------------ #
N = [1, 3]
P = [2]
I = N + P
L = ["a", "b", "c", "d", "f"]

if drop_c_d:
    L.remove("c")
    L.remove("d")

D       = 3
lam     = 0
mu      = {2: 1}
ghat    = {1: 1, 2: 1, 3: 1}

g = {
    (1, "a"): 0,    (1, "b"): 0,    (1, "c"): 0,    (1, "d"): 0,    (1, "f"): 1,
    (2, "a"): 0.5,  (2, "b"): 0.5,  (2, "c"): 0.75, (2, "d"): 0.75, (2, "f"): 1,
    (3, "a"): 0.25, (3, "b"): 0.25, (3, "c"): 0.5,  (3, "d"): 0.5,  (3, "f"): 1,
}

# ------------------------------------------------------------------ #
# Model                                                                #
# ------------------------------------------------------------------ #
model = Model()
model.setIntParam("display/verblevel", 0)

# --- Variables ---
# z_l in [0,1] continuous
z = {l: model.addVar(name=f"z_{l}", lb=0, ub=1, vtype=vartype_z) for l in L}

# gamma_i in [0,1]
gamma = {i: model.addVar(name=f"gamma_{i}", lb=0, ub=1, vtype="CONTINUOUS") for i in I}

# --- Objective ---
# min  sum_{i in N} gamma_i  -  sum_{i in P} mu_i * gamma_i
#      + lambda * (1 + sum_l z_l)
obj = (
    sum(gamma[i] for i in N)
    - sum(mu[i] * gamma[i] for i in P)
    + lam * (1 + sum(z[l] for l in L))
)
model.setObjective(obj, sense="minimize")

# --- Constraints ---

# (pos) for l in L, i in P:
#   gamma_i <= ghat_i - (ghat_i - g_{il}) * z_l
for l in L:
    for i in P:
        model.addCons(
            gamma[i] <= ghat[i] - (ghat[i] - g[i, l]) * z[l],
            name=f"pos_{i}_{l}"
        )

# (neg) for l in L, i in N:
#   gamma_i >= g_{il} - sum_{l': g_{il'} < g_{il}} (g_{il} - g_{il'}) * z_{l'}
for l in L:
    for i in N:
        g_il = g[i, l]
        dominated = [lp for lp in L if g[i, lp] < g_il]
        rhs = g_il - sum((g_il - g[i, lp]) * z[lp] for lp in dominated)
        model.addCons(gamma[i] >= rhs, name=f"neg_{i}_{l}")

# (complexity) 1 <= sum_l z_l <= D
# model.addCons(sum(z[l] for l in L) >= 1, name="complex_lb")
model.addCons(sum(z[l] for l in L) <= D, name="complex_ub")
#
model.addCons(z["a"] == 1/2, name="complex_ub")
model.addCons(z["b"] == 1/2, name="complex_ub")
model.addCons(z["c"] == 1/4, name="complex_ub")
model.addCons(z["d"] == 0, name="complex_ub")
# model.addCons(z["f"] == 0, name="complex_ub")


# # one per clause
# model.addCons(z["a"] + z["c"] + z["f"] <= 1, name="complex_lb")
# model.addCons(z["b"] + z["d"] <= 1)


# model.addCons(z["a"] + z["b"] >= 1)
# model.addCons(z["c"] + z["d"] + z["f"] == 0 )
# model.addCons(gamma[2] <= 1/2) 

# model.addCons(z["a"] + z["b"] == 0)
# model.addCons(gamma[3] >= 1/2)
#
# model.addCons(z["a"] + z["b"] >= 1)
# model.addCons(z["c"] + z["d"] + z["f"] >=1)
# model.addCons(gamma[2] <= 1/2)


# model.addCons(z["a"] + z["b"] >= 1)
# model.addCons(gamma[2] <= 1/2)


model.optimize()

status = model.getStatus()
print(f"Status : {status}")

if status == "optimal":
    print(f"Obj    : {model.getObjVal():.6f}")

    print("\nz variables:")
    for l in L:
        print(f"  z_{l} = {model.getVal(z[l]):.6f}")

    print("\ngamma variables:")
    for i in I:
        print(f"  gamma_{i} = {model.getVal(gamma[i]):.6f}")