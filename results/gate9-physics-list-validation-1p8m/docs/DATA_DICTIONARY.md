# Dataset dictionary

| File | Meaning |
|---|---|
| `run_metrics.csv` | one row per completed GATE run: unique seed, energy, physics config, primary count, peak depth, distal R80/R20, width and scored Edep per primary |
| `group_statistics.csv` | five metrics per (physics config, energy) group; 3-run mean, SD, SEM and Student t confidence interval |
| `physics_differences.csv` | differences in metric means between each hadronic list and EM reference, with combined SEM |
| `baseline_vs_new_em.csv` | difference between earlier single-run reference results and independent 3-seed EM means |
| `analysis_status.json` | count of complete runs, groups, problems and uncertainty metadata |

All Edep quantities are scored *within the configured water box*, not validated absorbed dose in Gy. Depth-related columns are in millimeters.

Raw `.mhd`/`.raw` dose volumes are not distributed in this package. Preserve them on your WSL drive if additional per-voxel analyses are planned.
