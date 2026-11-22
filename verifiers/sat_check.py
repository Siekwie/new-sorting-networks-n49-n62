"""SAT check: is there a 0/1 input whose output has out[i]=1, out[i+1]=0 for some i?
UNSAT  => the network sorts (0-1 principle). No prefix argument is used.
Usage: python sat_check.py <network.txt> <n>
"""
import re, sys, time
from pysat.solvers import Cadical153

path, n = sys.argv[1], int(sys.argv[2])
net = [(int(a), int(b)) for a, b in re.findall(r"\((\d+)\s*,\s*(\d+)\)", open(path).read())]
nv = n
cur = list(range(1, n + 1))
cls = []
for a, b in net:
    x, y = cur[a], cur[b]
    lo, hi = nv + 1, nv + 2; nv += 2
    # lo <-> x & y
    cls += [[-lo, x], [-lo, y], [lo, -x, -y]]
    # hi <-> x | y
    cls += [[hi, -x], [hi, -y], [-hi, x, y]]
    cur[a], cur[b] = lo, hi
sel = []
for i in range(n - 1):
    nv += 1; s = nv; sel.append(s)
    cls += [[-s, cur[i]], [-s, -cur[i + 1]]]
cls.append(sel)
t = time.time()
with Cadical153(bootstrap_with=cls) as S:
    r = S.solve()
    if r:  # confirm the counterexample by direct simulation
        m = set(l for l in S.get_model() if l > 0)
        v = [1 if (i + 1) in m else 0 for i in range(n)]
        for a, b in net:
            if v[a] > v[b]: v[a], v[b] = v[b], v[a]
        assert any(v[i] > v[i + 1] for i in range(n - 1)), "model does not reproduce"
    print(f"{path.split('/')[-1]}: vars={nv} clauses={len(cls)} -> "
          f"{'SAT (UNSORTED input exists)' if r else 'UNSAT (sorts)'} in {time.time()-t:.1f}s", flush=True)
