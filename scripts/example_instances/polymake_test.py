from itertools import chain, combinations

def powerset_without_empty(iterable):
    s = list(iterable)
    return chain.from_iterable(combinations(s, r) for r in range(1, len(s)+1))

n=5
gamma = [0,1,3,5,9,12]
rows = []

for l in range(1, n+1):
    for nonempty_subset in powerset_without_empty(range(1,l)):
        row = [0]*(n+2)
        row[0] = gamma[l]
        row[n+1] = -1
        for idx, i in enumerate(nonempty_subset):
            next_i = nonempty_subset[idx + 1] if idx + 1 < len(nonempty_subset) else l
            row[i] = -(gamma[next_i] - gamma[i])
        for idx,eta in enumerate(range(l+1, n+1)):
            row[eta] = gamma[eta]-gamma[l]
        rows.append(row)

rows = sorted(rows, reverse=True)
print(",\n".join([str(row) for row in rows]), end = "")