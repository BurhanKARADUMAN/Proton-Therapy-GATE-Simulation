# Example outputs and reproducibility

Generated on 13 November 2026 with `python -m braggkit.cli --output-dir examples --histories 2000` using a fixed RNG seed (documented in source). These are **synthetic illustrations** from the simplified residual-energy proxy, never clinical absorbed dose, experimental measurements, or GATE Monte Carlo results.

- `01_energy_sweep.png`: nominal 70, 100, 130, 150, 180 and 200 MeV proton energy settings.
- `02_medium_comparison.png`: homogeneous illustrative relative stopping-power ratios (water=1, lung-like=0.3, bone-like=1.6) with **no validated CT/RSP calibration**.
- `03_uncertainty.png`: different Gaussian incident-energy spread parameters; sample-based variability is not a full systematic uncertainty budget.
- `04_sobp_optimization.png`: 120–152 MeV layers combined with constrained nonnegative fitting in a 105–155 mm interval. Target flatness in this toy is not a treatment-plan metric.
- `energy_sweep.csv`, `sobp_layers.csv`, `demo_metadata.json`: numerical outputs sufficient to inspect the generated figures and settings.

To regenerate:

```bash
python -m braggkit.cli --output-dir examples --histories 2000
```
