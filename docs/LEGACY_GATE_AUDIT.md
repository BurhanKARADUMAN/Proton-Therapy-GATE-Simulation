# Audit of the original GATE project (November 2026)

This report records **issues visible in repository files**, not the outcomes of any independently executed GATE simulation.

1. **Bone label incorrectly maps to water.** `hasta_yarat.py` encodes air=0, tissue=1, bone=2, but `hasta_materyal.dat` contains `1.5 2.5 Water`. Therefore the model has *no bone material contrast* in GATE. The file also appears to lack the initial count line required for GATE's range-translator format. Confirm the exact version/syntax before running.
2. **Raw dose payload missing.** The uploaded `doz_haritasi-Dose.mhd` references `doz_haritasi-Dose.raw`, but that raw data file is absent in the GitHub tree. This makes the 3D dose distribution irreproducible from the public snapshot alone.
3. **Output-name mismatch.** `hasta_doz_oku.py` looks for `hasta_doz-Dose.mhd`; the repository contains only `doz_haritasi-Dose.mhd` (apparently from another simulation). The analyst must verify that patient and dose files correspond to the same run.
4. **Voxel resolution coarse for distal falloff.** `hasta_yarat.py` sets 10 mm spacing and the voxelized 150 MeV phantom spans 50 cm per dimension. That cannot meaningfully resolve a Bragg distal 80–20% falloff on the few-mm scale.
5. **Archival event statistics.** `insan_sim.mac` specifies 500 Bq for 0.5 s and `klinik_sim.mac` 1000 Bq for 0.5 s, implying expected order 250 and 500 emitted primaries respectively for steady sources (subject to GATE source semantics). These are much too small for a precise dose map; no uncertainties are provided.
6. **Physics accuracy needs review.** Macros list `emstandard_opt3`; in therapeutic proton simulations, verify the hadronic interaction physics, production cuts, range-energy tables, and cross sections as appropriate for the exact GATE/Geant4 version.
7. **Anatomical proof overstated.** A synthetic phantom is not a patient study; a visually attractive overlay alone cannot establish a clinically significant 'severe' shift or the effectiveness of in-vivo dosimetry. Such statements require measured or fully validated numerical evidence.
8. **Coordinate sanity.** `klinik_sim.mac` launches the proton beam from +x toward -x while the old overlay script slices the z axis; this is not by itself wrong, but a longitudinal Bragg curve should be extracted **along x** and the scoring geometry must be checked for consistent registration.

## Controlled changes proposed for an actual GATE re-run

- Preserve the archived `.mac` files; **do not silently rewrite their history**.
- Use an explicit range-to-material table (example in `gate/label_to_material_EXAMPLE.dat`), with the material token for bone matching your installed `GateMaterials.db` exactly.
- Increase spatial resolution (e.g. 1–2 mm in beam direction), choose a clinically relevant phantom length and record detector/world placements to avoid overlap.
- Score both dose and statistical uncertainty; archive event counts, RNG seeds, source energy spectra, field sizes and all generated `.mhd/.raw` output pairs.
- Document expected dose units and reference frame; explicitly compare stopping depth and R80 to published or experimental benchmarks.
- Verify GATE macros *in your own installed version*; the example material table has not been executed against GATE here.
