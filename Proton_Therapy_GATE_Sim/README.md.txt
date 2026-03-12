# Monte Carlo Simulation of Proton Therapy: Range Uncertainty in Voxelized Phantoms

## 📌 Project Overview
This project demonstrates a complete end-to-end Monte Carlo simulation workflow for proton therapy using **GATE (Geant4 Application for Tomographic Emission)**. The primary objective is to visualize and analyze the **"Range Uncertainty"** problem caused by tissue density variations (e.g., lung cavities vs. soft tissue) and its effect on the **Bragg Peak** positioning.

## 🛠️ Tools & Technologies Used
* **Simulation Engine:** GATE / Geant4 (Linux environment)
* **Data Processing & Voxelization:** Python (NumPy, SimpleITK)
* **Dose Visualization:** Python (Matplotlib)

## 🚀 Methodology
1. **Synthetic Patient Generation (`hasta_yarat.py`):** Created a 3D synthetic voxelized phantom representing a human torso containing soft tissue (water equivalent), two lung cavities (air), and a spine (bone). The output is a `.mhd` medical image format.
2. **Material Calibration (`hasta_materyal.dat`):** Defined a Hounsfield Unit (HU) to physical material translation table for the GATE engine to accurately simulate particle scattering and stopping power.
3. **Monte Carlo Beam Execution (`klinik_sim.mac`):** Fired a 150 MeV broad proton beam targeting the lung-tissue interface using the `emstandard_opt3` physics list. Recorded the 3D dose distribution using a `DoseActor`.
4. **Dose Mapping & Analysis (`hasta_doz_oku.py`):** Overlaid the resulting radiation dose map transparently onto the anatomical CT matrix.

## 📊 Results & Conclusion
![Proton Therapy Dose Map](hasta_tedavi_plani.png)
*Figure: The resulting dose map overlay. Notice the severe curvature and distal shift of the Bragg Peak (red/yellow region) as protons travel through the low-density lung cavities compared to the dense soft tissue.*

This simulation visually proves the critical need for **in-vivo dosimetry** (such as Prompt Gamma Imaging or TOF-PET) during proton therapy. Without real-time tracking, anatomical heterogeneities cause protons to overshoot or undershoot the targeted tumor volume, highlighting the limitations of static treatment planning.