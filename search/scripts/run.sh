#!/bin/bash
# run.sh cfgbase seconds : runs SorterHunter with timestamps
cd "$(dirname "$0")"; start=$(date +%s)
timeout $2 ./SorterHunter.exe $1.cfg 2>&1 | while IFS= read -r line; do echo "[t=$(( $(date +%s)-start ))s] $line"; done > $1.log
