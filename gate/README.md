# Correcting materials before GATE execution

The archived `hasta_materyal.dat` assigns `2` (bone) to `Water`. `label_to_material_EXAMPLE.dat` provides a **candidate** correct label-to-material mapping. The token `SpineBone` is used in a GATE documentation example and may not be defined in your installed `GateMaterials.db`; replace it with a confirmed local bone material. Verify your exact GATE version's handling of comment lines and `ImageNestedParametrisedVolume` before running. This file is **not validated by a GATE execution**.

The existing source is saved in `Proton_Therapy_GATE_Sim/`; leave it unchanged until reconstructing the original simulation conditions. A proper GATE campaign will require its own validated macros, consistent geometry/voxel sizes, physics processes, statistics and complete output files.
