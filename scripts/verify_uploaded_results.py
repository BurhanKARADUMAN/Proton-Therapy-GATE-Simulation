#!/usr/bin/env python3
"""Verify the Edep/primary CSVs and print derived summary; no Geant4 execution."""
from pathlib import Path
import csv

def main():
    d = Path(__file__).resolve().parents[1] / 'results' / 'gate9-water-50k' / 'data'
    with (d/'summary_original.csv').open(newline='') as f:
        rows=list(csv.DictReader(f))
    for r in rows:
        e=int(r['energy_MeV'])
        p=d/f'E{e:03d}_N50000_R0_profile.csv'
        with p.open(newline='') as f:
            data=list(csv.DictReader(f))
        values=[float(x['edep_MeV_per_primary_per_1mm_bin']) for x in data]
        depths=[float(x['depth_mm']) for x in data]
        assert len(values)==400 and len(depths)==400, f'unexpected bin count for {e}'
        assert all(x>=0 for x in values), f'negative Edep for {e}'
        assert all(abs((depths[i+1]-depths[i])-1)<1e-8 for i in range(len(depths)-1))
        total=sum(values)
        assert 0.98*e < total <= 1.01*e, f'energy sum inconsistent for {e}: {total}'
        print(f'{e:3d} MeV: integrated Edep = {total:.5f} MeV/primary; nominal {e} MeV')
    print('CSV input checks passed (these are data-integrity checks, not physics validation).')

if __name__ == '__main__':
    main()
