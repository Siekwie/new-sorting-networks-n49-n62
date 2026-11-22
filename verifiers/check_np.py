"""Independent exact checker (numpy, no 64-vector batching).

Usage: python check_np.py <network.txt> <n> [cap=16]
- format: every line is one layer; no channel twice within a line; 0 <= i < j < n
- size, ASAP depth, and whether the file's lines equal the ASAP layering
- exact 0-1 proof: maximal commuting prefix (comparator joins only if neither channel
  was touched by a non-prefix comparator, union-find groups <= cap), exhaustive group
  output sets, then the remaining comparators on the whole Cartesian product at once.
"""
import itertools, re, sys
import numpy as np

path, n = sys.argv[1], int(sys.argv[2])
cap = int(sys.argv[3]) if len(sys.argv) > 3 else 16

lines = [l for l in open(path).read().splitlines() if l.strip()]
layers = [[(int(a), int(b)) for a, b in re.findall(r"\((\d+)\s*,\s*(\d+)\)", l)] for l in lines]
net = [c for L in layers for c in L]
ok_fmt = True
for k, L in enumerate(layers):
    ch = [x for c in L for x in c]
    if len(ch) != len(set(ch)):
        ok_fmt = False; print(f"line {k+1}: channel repeated")
for a, b in net:
    if not (0 <= a < b < n):
        ok_fmt = False; print("bad comparator", a, b)

free = [0] * n; asap = []
for a, b in net:
    d = max(free[a], free[b]); free[a] = free[b] = d + 1; asap.append(d)
depth = max(free)
file_is_asap = all(asap[i] == k for i, k in
                   zip(range(len(net)), [k for k, L in enumerate(layers) for _ in L]))

parent = list(range(n)); gsize = [1] * n
def find(x):
    while parent[x] != x:
        parent[x] = parent[parent[x]]; x = parent[x]
    return x
touched = [False] * n; prefix, rest = [], []
for a, b in net:
    ra, rb = find(a), find(b)
    if not touched[a] and not touched[b] and (ra == rb or gsize[ra] + gsize[rb] <= cap):
        if ra != rb:
            parent[ra] = rb; gsize[rb] += gsize[ra]
        prefix.append((a, b))
    else:
        rest.append((a, b)); touched[a] = touched[b] = True

groups = {}
for c in range(n):
    groups.setdefault(find(c), []).append(c)
groups = list(groups.values())

def outputs(chs):
    k = len(chs); idx = {c: i for i, c in enumerate(chs)}
    comps = [(idx[a], idx[b]) for a, b in prefix if a in idx]
    X = ((np.arange(1 << k)[:, None] >> np.arange(k)) & 1).astype(bool)
    for i, j in comps:
        lo = X[:, i] & X[:, j]; hi = X[:, i] | X[:, j]; X[:, i] = lo; X[:, j] = hi
    return np.unique(X, axis=0)

outs = [outputs(g) for g in groups]
total = 1
for o in outs: total *= len(o)
# build product as an (total x n) bool matrix
V = np.zeros((total, n), dtype=bool)
reps = total
tile = 1
for g, o in zip(groups, outs):
    reps //= len(o)
    block = np.repeat(o, reps, axis=0)          # each row repeated reps times
    block = np.tile(block, (tile, 1))
    V[:, g] = block
    tile *= len(o)
assert len(np.unique(V, axis=0)) == total
for i, j in rest:
    lo = V[:, i] & V[:, j]; hi = V[:, i] | V[:, j]; V[:, i] = lo; V[:, j] = hi
unsorted = int(np.count_nonzero((V[:, :-1] & ~V[:, 1:]).any(axis=1)))
print(f"{path.split('/')[-1]}: n={n} size={len(net)} depth={depth} lines={len(layers)} "
      f"format_ok={ok_fmt} file_is_asap={file_is_asap} prefix={len(prefix)} "
      f"groups={[len(g) for g in groups]} outs={[len(o) for o in outs]} "
      f"product={total} checked={total} unsorted={unsorted} -> {'SORTS' if unsorted == 0 and ok_fmt else 'FAIL'}")
