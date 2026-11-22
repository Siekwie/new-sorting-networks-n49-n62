#!/bin/bash
# reproduce n=57 453-CE find: SorterHunter v0.4 (predecessor's Windows build), config repro57.cfg (seed 1)
cd "$(dirname "$0")"
start=$(date +%s)
timeout 2400 ./SorterHunter.exe repro57.cfg 2>&1 | while IFS= read -r line; do echo "[t=$(( $(date +%s)-start ))s] $line"; done > repro57.log
