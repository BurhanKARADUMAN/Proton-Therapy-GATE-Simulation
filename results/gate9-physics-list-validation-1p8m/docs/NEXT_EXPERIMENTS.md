# Planned high-value next experiments

## Priority 1 — Grid convergence (without changing physical meaning)

At 150 and 200 MeV, compare longitudinal voxel sizes of 1 mm, 0.5 mm, 0.25 mm; keep scoring dimensions and water phantom length consistent. Quantify peak-bin depth, distal R80, R20 and 80–20 falloff against bin size. Use independent seeds and sufficient histories; do not call interpolation precision "accuracy".

## Priority 2 — Particle/energy bookkeeping

Add scoring of secondary neutron, photon, proton, recoil and escaping particle types where supported by installed GATE/Geant4. Calculate incident kinetic energy, energy deposited within water and energy transported out of the sensitive region. Track contributions by particle type and process when possible. This tests the mechanisms behind deposited-energy differences.

## Priority 3 — Matched EM configurations

Compare reference lists under explicitly chosen/verified electromagnetic configurations. Confirm physics-list settings from the compiled Geant4 11.2.2 and GATE 9.4 logs, not only from nominal list names.

## Priority 4 — External benchmark

Benchmark distal R80 consistently against measured water-phantom depth-dose data, and compare *appropriate* range measures with NIST PSTAR evaluated ranges. Do not treat CSDA range as identical to R80.

## Priority 5 — Monte Carlo uncertainty

Increase independent repeats and history counts to characterize run-to-run variability; report confidence intervals with clear sampling unit and all relevant systematic uncertainty budgets.
