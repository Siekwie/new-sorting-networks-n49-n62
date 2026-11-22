# New sorting networks for n = 49 to 62

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.24442070.svg)](https://doi.org/10.5281/zenodo.24442070)

Eleven sorting networks that are not dominated by any entry in Bert Dobbelaere's
[extended list of sorting networks](https://bertdobbelaere.github.io/sorting_networks_extended.html)
as of its version "Page updated on Fri Nov 20 20:53:00 2026" (fetched 2026-11-22).
Five of them have fewer comparators than any listed network for the same number of inputs.

Size means the number of comparators (CEs). Depth means the number of layers when every
comparator is placed in the earliest layer in which both of its channels are free.

## Results

### New size upper bounds (one comparator below the smallest listed size)

| n  | size | depth | file | listed pairs (size, depth) |
|----|-----:|------:|------|----------------------------|
| 50 | **375** | 24 | [n50_375_24.txt](networks/n50_375_24.txt) | (376,21) (378,20) (381,19) |
| 55 | **431** | 22 | [n55_431_22.txt](networks/n55_431_22.txt) | (432,20) (447,19) |
| 57 | **453** | 24 | [n57_453_24.txt](networks/n57_453_24.txt) | (454,21) (461,20) |
| 58 | **464** | 24 | [n58_464_24.txt](networks/n58_464_24.txt) | (465,21) (470,20) |
| 61 | **496** | 24 | [n61_496_24.txt](networks/n61_496_24.txt) | (497,22) (498,21) (502,20) |

### New (size, depth) pairs

| n  | size | depth | file | improves on |
|----|-----:|------:|------|-------------|
| 49 | 368 | 20 | [n49_368_20.txt](networks/n49_368_20.txt) | new point between (365,21) and (369,19) |
| 57 | 457 | 20 | [n57_457_20.txt](networks/n57_457_20.txt) | listed (461,20) |
| 60 | 487 | 20 | [n60_487_20.txt](networks/n60_487_20.txt) | listed (488,20) |
| 61 | 497 | 21 | [n61_497_21.txt](networks/n61_497_21.txt) | listed (497,22) and (498,21) |
| 61 | 501 | 20 | [n61_501_20.txt](networks/n61_501_20.txt) | listed (502,20) |
| 62 | 510 | 20 | [n62_510_20.txt](networks/n62_510_20.txt) | listed (511,20) |

The files use the table's own format: one layer per line, each line a list of pairs
`[(i,j),(k,l),...]` with `i < j`, channels numbered from 0. The listed pairs for every n
are in [table/snapshot_2026-11-20.md](table/snapshot_2026-11-20.md).

## How they were found

The networks came out of an automated AI-agent research pipeline that ran overnight
on 2026-11-21/22. The pipeline used SorterHunter and the networks from Dobbelaere's
own list as raw material.

- **n = 50, 57, 58 (size).** SorterHunter v0.4, with the four best-size group sorters of the
  listed network as `FixedPrefix` and its merge tail as `InitialNetwork`. The groups are
  12/13/12/13 channels for n=50 (from (376,21)), 12/13/16/16 for n=57 (from (454,21)) and
  10/16/16/16 for n=58 (from (465,21)). Settings: `EscapeRate=500`, `MaxMutations=2`,
  `PrefixType=1`, `Symmetric=0`, default weights. Seeds and times to the final network:
  n=50 seed 7 at 1036 s, n=57 seed 1 at 839 s, n=58 seed 7 at 1990 s.
  Configs and logs are in [search/n50](search/n50), [search/n57](search/n57) and [search/n58](search/n58).
- **n = 55 (size).** Same recipe, seeded with the best-size sorters on the 15/16/8/16 groups
  of the listed (432,20) network plus its 237-CE merge tail, seed 7. (431,24) appeared at
  t=344 s and (431,22) at t=374 s. See [search/n55](search/n55).
- **n = 61 (size).** A new starting network. Take the listed 63-channel (515,21) network,
  fix the top two inputs of its 15-channel group to 1, remove the comparators that become
  redundant and untangle the lines, then put a best-size 13-channel sorter on the remaining
  13 channels. This gives the (497,21) network above with no search at all. SorterHunter on
  that seed (seed 7) reached (496,24) at t=1136 s. See [search/n61](search/n61).
- **Depth-20 networks (n = 49, 57, 60, 61, 62).** The listed depth-21 networks for these n start
  with sorters on four disjoint channel groups. Replacing those sorters with known
  minimum-depth sorters of the same width costs one comparator per group and saves one layer:
  12 channels (39,9)→(40,8), 13 channels (45,10)→(46,9), 15 channels (56,10)→(57,9),
  16 channels (60,10)→(61,9). Every sorter on k channels has the same output set, so the rest
  of the network still sorts. For n=61 the base is the (497,21) network above. No search is
  involved. See [search/depth20](search/depth20).

Wall-clock times were measured with about 8 to 11 search jobs sharing 8 cores.

SorterHunter was built from upstream commit `89518e8af10bcb5f56824323da68c2f69618358d`
(v0.4) with small portability changes only: `%zu` printf formats for MinGW, unbuffered
stdout, and an optional time limit read from `SH_TIMELIMIT`. The search code is unchanged.
The diff is [search/sorterhunter/portability.patch](search/sorterhunter/portability.patch).

The scripts in [search/scripts](search/scripts) and [search/depth20](search/depth20) are
copied as they were run. They expect SorterHunter's `Networks/Sorters` JSON files and a saved
copy of the table page in their working directory, which are not redistributed here.

## Verification

Every network was checked by seven programs written separately:

| # | program | method |
|---|---------|--------|
| 1 | [rv_prefix_product.c](verifiers/rv_prefix_product.c) | prefix product, group cap 16 |
| 2 | [rv_prefix_product_cap24.c](verifiers/rv_prefix_product_cap24.c) | prefix product, group cap 24 (finds a different prefix for some n) |
| 3 | [vf.c](verifiers/vf.c) | prefix product, bit-sliced |
| 4 | [verify_swap.c](verifiers/verify_swap.c) | prefix product |
| 5 | [verify_A1.cpp](verifiers/verify_A1.cpp) | prefix product |
| 6 | [check_np.py](verifiers/check_np.py) | prefix product in numpy, all vectors at once (no 64-vector batching), plus format checks |
| 7 | [sat_check.py](verifiers/sat_check.py) | SAT (CaDiCaL via python-sat), no prefix argument |
| 8 | [rv_layer_cut.c](verifiers/rv_layer_cut.c) | prefix = first 4 layers only, 1e8 to 1.3e9 vectors (slow, optional) |

**Prefix product (an exact proof by the 0-1 principle).** A comparator goes into the
prefix only if neither of its channels has been touched by an earlier comparator outside
the prefix. Such a comparator commutes with every earlier non-prefix comparator, so moving
the prefix to the front does not change the network. The prefix falls apart into
sub-networks on disjoint channel groups. Each group's exact output set is computed over all
2^|g| inputs of 0s and 1s, and the rest of the network is run on every vector of the
Cartesian product of those sets. If all of them come out sorted, every 0/1 input is sorted,
and by the 0-1 principle the network sorts every input. Product sizes are 30,758 to
73,984 vectors.

**SAT.** Each comparator is encoded as min = AND and max = OR. The solver is asked for a 0/1
input with `out[i] = 1` and `out[i+1] = 0` for some i. UNSAT means the network sorts. This
check does not use the prefix argument. All 11 networks are UNSAT. A satisfying assignment
for a broken network is replayed through the network to confirm it.

**Checks of the checkers.**
- Two broken networks ([controls](networks/controls)), each with one comparator removed,
  fail under every program.
- In 40 further mutants (one tail comparator deleted at random from the n=55, 57, 60 and 61
  networks), the SAT check and the numpy check gave the same answer every time.
- `check_np.py` confirms that no channel appears twice within a line, that `0 <= i < j < n`,
  that each file's lines are exactly the ASAP layering, and that the number of vectors
  checked equals the full product.
- [random_test.py](verifiers/random_test.py) runs 20,000 random inputs (real-valued, and
  small integers with repeats) through each full network. It found no failures. This is
  only a sanity check, not part of the proof.

To rerun everything (needs gcc, g++, Python 3 with `numpy` and `python-sat`):

```sh
bash verifiers/run_all.sh | tee verifiers/results.txt
RUN_LAYER_CUT=1 bash verifiers/run_all.sh     # also runs the slow verifier 8
```

The output of the last run is in [verifiers/results.txt](verifiers/results.txt).

## Licence

Code (verifiers and scripts): MIT, see [LICENSE](LICENSE). Networks, configs and logs:
CC BY 4.0. The networks build on networks from Bert Dobbelaere's list and on
[SorterHunter](https://github.com/bertdobbelaere/SorterHunter) (MIT).

## Citation

Jannik Wiest, *New sorting networks for n = 49 to 62*, v1.0.0, Zenodo, 2026.
DOI [10.5281/zenodo.24442070](https://doi.org/10.5281/zenodo.24442070)
(all versions: [10.5281/zenodo.24442069](https://doi.org/10.5281/zenodo.24442069)).
See also [CITATION.cff](CITATION.cff).
