#!/usr/bin/env python3
"""Verify expected uploaded CSV structure and key numerical relationships."""
import csv, json, math
from pathlib import Path
from collections import defaultdict
from statistics import mean, stdev

ROOT=Path(__file__).resolve().parent
DATA=ROOT/'data'

def read(name):
    with (DATA/name).open(newline='',encoding='utf-8') as f:
        return list(csv.DictReader(f))

def check():
    runs=read('run_metrics.csv')
    groups=read('group_statistics.csv')
    diffs=read('physics_differences.csv')
    status=json.loads((DATA/'analysis_status.json').read_text(encoding='utf-8'))
    assert len(runs)==36 and sum(int(x['primaries']) for x in runs)==1_800_000
    assert len(set(x['seed'] for x in runs))==36
    combos=defaultdict(list)
    for x in runs:
        key=(x['physics'],int(x['energy_MeV']))
        combos[key].append(x)
        assert int(x['primaries'])==50_000
        for column in ('distal_R80_mm','distal_R20_mm','distal_80_20_falloff_mm','edep_MeV_per_primary'):
            assert math.isfinite(float(x[column])), (key,column)
    assert len(combos)==12 and all(len(v)==3 for v in combos.values())
    assert len(groups)==60 and len(diffs)==40
    assert status['complete_runs']==36 and not status['problems']
    for s in groups:
        vals=[float(x[s['metric']]) for x in combos[s['physics'],int(s['energy_MeV'])]]
        assert math.isclose(mean(vals),float(s['mean']),rel_tol=1e-9,abs_tol=1e-8)
        assert math.isclose(stdev(vals),float(s['sd']),rel_tol=1e-8,abs_tol=1e-8)
    for d in diffs:
        q=lambda p: next(float(s['mean']) for s in groups if s['physics']==p and s['energy_MeV']==d['energy_MeV'] and s['metric']==d['metric'])
        assert math.isclose(q(d['comparison'])-q(d['reference']),float(d['mean_difference']),rel_tol=1e-8,abs_tol=1e-8)
    print('PASS: 36 unique-seed runs; 1,800,000 proton histories; 12 groups; 60 statistics; 40 physics differences')

if __name__=='__main__':check()
