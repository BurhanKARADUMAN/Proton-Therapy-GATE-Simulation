# Physics model, algorithms and validation boundary

## What is calculated?

A water-equivalent *residual-energy surrogate*, not a Geant4 track and not absorbed dose. For an input proton energy \(E_0\), a toy range \(R = 10 a E_0^p\) in millimetres is used, with \(a=0.0022\), \(p=1.77\). This illustrative fit may deviate from NIST PSTAR depending on energy. Relative stopping-power scaling is applied to a uniform medium; heterogeneous geometry is **not** treated.

At each depth \(z\), define \(W(z)=\rho_{\rm rel}z\) and:

\[E_{\rm remaining}(z) = E_0\max\left(1-\frac{W(z)}{R},0\right)^{1/p}.\]

Deposited-energy *proxy* in a scoring bin is \(\Delta E=E_{\rm remaining}(z_i)-E_{\rm remaining}(z_{i+1})\). Divide by bin width, average over sampled primaries. The mean model integrates back to the incident energy **only because nuclear losses and escaping secondaries have been omitted**. This is not a check of physical energy deposition in realistic media.

## Uncertainty

Toy histories independently sample Gaussian incident energy spread and Gaussian residual range perturbations. Standard errors are computed from sample variance across toy histories. These are **not** uncertainties of GATE transport or model bias. Any study should separate beam energy uncertainty, material calibration/RSP, range straggling, nuclear modeling, detector noise and statistical counting uncertainty.

## Distal range markers

After the global maximum, an interpolated crossing with a fractional peak height defines distal R90 / R80 / R20. These are simple monoenergetic-peak metrics; a complex SOBP with multiple local maxima may require a different dose prescription and plateau convention.

## SOBP example

A matrix of pristine energy-layer proxies is fitted with nonnegative least squares on a demonstration depth interval; the layer weights are normalized to sum to 1. No objective is imposed on dose outside the target, no lateral conformity is modeled, no biological dose is estimated, and the algorithm is **not** a treatment-plan optimizer.

## Numerical benchmarking plan

1. For energies 70, 100, 150, 180, and 200 MeV, extract evaluated NIST PSTAR water CSDA and projected ranges, plus stopping-power curves. Compare with this toy model and quantify relative errors.
2. Model a well-defined homogeneous water box in GATE and use a suitable Geant4 physics list and source definition; validate **actual dose and uncertainty scoring** with sufficient histories.
3. Make the beam-axis profile with `bragg-demo --mhd ... --axis x --sign -1`; verify voxel spacing, origin, extent, and mass/energy output units.
4. Report peak depth, distal R80, distal 80–20 falloff and statistics; test convergence with voxel size and histories.
5. Only then introduce calibrated heterogeneities and material-specific range shift; validate against a tissue-equivalent phantom.

No benchmark is claimed completed by this V2 code package.
