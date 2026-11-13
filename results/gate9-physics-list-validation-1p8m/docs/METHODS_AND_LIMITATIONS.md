# Methods, metrics, and uncertainty accounting

## Geometry and source

Campaign metadata (from the archived campaign script and shared analysis outputs): monoenergetic protons at 70/100/150/200 MeV incident along −x on a 40 cm water box. Longitudinal scoring grid: 400 bins × 1 mm; Edep recorded by GATE DoseActor. Each (energy, physics list) group contains three unique seeds with 50,000 primaries per seed.

## Analysis code semantics

The supplied `analyze_campaign.py` reverses the raw 400-point array to align depth with the beam entrance (`[::-1]`), then defines:

- `peak_depth_mm` = midpoint of the highest Edep depth bin
- `distal_R80_mm` = first descending crossing after the highest bin at 80% of that bin's Edep, linearly interpolated between *bin centers*
- `distal_R20_mm` = analogous 20% crossing
- `distal_80_20_falloff_mm` = R20 − R80
- `edep_MeV_per_primary` = sum of the scored Edep vector / primary count

### Aggregate statistics

- `mean` = arithmetic mean over independent run-level metrics (n=3)
- `sd` = unbiased sample standard deviation across runs
- `sem` = SD / sqrt(n)
- `ci95_lower` / `ci95_upper` = mean ± t_{0.975,n−1} × SEM (when n≥2)
- `physics_differences.combined_sem` = sqrt(SEM_A² + SEM_B²) for independent group means, **not** a confidence interval and not a substitute for systematic validation

## Sampling precision vs spatial accuracy

The low across-run SD for R80 reflects the stability of extracting crossings from a 1 mm grid under the chosen model. It is not a guaranteed spatial accuracy. Change scoring resolution and physics configuration to quantify systematic stability.

## Plot normalization

Each of `profiles_E*.png` was normalized to that physics list/energy ensemble's own peak amplitude, so the peaks all appear near 1.0. Comparisons of absolute energy deposited require `edep_MeV_per_primary` or non-normalized profiles.

## Reference guidance

Reference hadronic lists such as `QGSP_BIC` and `FTFP_BERT` include electromagnetic and hadronic components and may differ in default EM constructors. The phrase *hadronic-list sensitivity* is therefore more rigorous than *the isolated effect of nuclear interactions*.

All results are educational/nonclinical; do not use them for patient-specific dose planning.
