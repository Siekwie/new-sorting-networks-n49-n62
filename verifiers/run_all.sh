#!/bin/bash
# Build every verifier and run it on every network and on the two broken controls.
# Needs gcc, g++, python 3 with numpy and python-sat (pip install numpy python-sat).
# Run from the repository root:  bash verifiers/run_all.sh | tee verifiers/results.txt
set -u
cd "$(dirname "$0")/.."
B=build; mkdir -p $B
gcc -O2 -o $B/rv_prefix_product      verifiers/rv_prefix_product.c
gcc -O2 -o $B/rv_prefix_product_cap24 verifiers/rv_prefix_product_cap24.c
gcc -O2 -o $B/rv_layer_cut           verifiers/rv_layer_cut.c
gcc -O2 -o $B/vf                     verifiers/vf.c
gcc -O2 -o $B/verify_swap            verifiers/verify_swap.c
g++ -O2 -o $B/verify_A1              verifiers/verify_A1.cpp

nets="n49_368_20 n50_375_24 n55_431_22 n57_453_24 n57_457_20 n58_464_24 n60_487_20 n61_496_24 n61_497_21 n61_501_20 n62_510_20"
for b in $nets controls/broken57_452 controls/broken61_495; do
  f=networks/$b.txt; base=$(basename $b); n=${base:1:2}
  case $base in broken57*) n=57;; broken61*) n=61;; esac
  echo "=== $b (n=$n)"
  echo "[1 rv_prefix_product, cap 16]";  $B/rv_prefix_product $f $n 16 | tail -1
  echo "[2 rv_prefix_product_cap24]";    $B/rv_prefix_product_cap24 $f $n | tail -1
  echo "[3 vf, cap 16]";                 $B/vf $f 16 | tail -1
  echo "[4 verify_swap, cap 16]";        $B/verify_swap $n $f 16 | tail -1
  echo "[5 verify_A1, cap 16]";          $B/verify_A1 $n $f 16 | tail -1
  echo "[6 check_np.py, cap 16]";        python verifiers/check_np.py $f $n 16 | tail -1
  echo "[7 sat_check.py]";               python verifiers/sat_check.py $f $n | tail -1
  echo "[random_test.py, sanity only]";  python verifiers/random_test.py $f $n | tail -1
  if [ "${RUN_LAYER_CUT:-0}" = 1 ]; then  # slow: 1e8..1.3e9 vectors per network
    echo "[8 rv_layer_cut, 4 layers]";   $B/rv_layer_cut $n $f 4 | tail -1
  fi
done
