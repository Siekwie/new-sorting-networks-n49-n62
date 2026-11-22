"""Sanity check only (not a proof): run random inputs through the full network.
Usage: python random_test.py <network.txt> <n> [trials=20000]
Half the trials are random reals, half are small integers with many repeats.
"""
import random, re, sys

path, n = sys.argv[1], int(sys.argv[2])
trials = int(sys.argv[3]) if len(sys.argv) > 3 else 20000
net = [(int(a), int(b)) for a, b in re.findall(r"\((\d+)\s*,\s*(\d+)\)", open(path).read())]
rng = random.Random(12345)
bad = 0
for t in range(trials):
    v = [rng.random() for _ in range(n)] if t % 2 == 0 else [rng.randrange(4) for _ in range(n)]
    for a, b in net:
        if v[a] > v[b]:
            v[a], v[b] = v[b], v[a]
    bad += any(v[i] > v[i + 1] for i in range(n - 1))
print(f"{path.split('/')[-1]}: {trials} random inputs, {bad} unsorted")
