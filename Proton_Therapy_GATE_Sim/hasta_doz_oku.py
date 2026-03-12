import SimpleITK as sitk
import matplotlib.pyplot as plt
import numpy as np

print("Hasta CT'si ve Tedavi Dozu Birlestiriliyor...")
# CT'yi Oku
ct_image = sitk.ReadImage("hasta_bt.mhd")
ct_matrix = sitk.GetArrayFromImage(ct_image)

# Dozu Oku
dose_image = sitk.ReadImage("hasta_doz-Dose.mhd")
dose_matrix = sitk.GetArrayFromImage(dose_image)

orta_kesit_z = ct_matrix.shape[0] // 2

plt.figure(figsize=(10, 8))
# 1. Altta Hastanin CT'sini (Siyah Beyaz) Ciz
plt.imshow(ct_matrix[orta_kesit_z, :, :], cmap='gray', interpolation='none')

# 2. Uste Radyasyon Dozunu (Renkli) Ciz
dose_slice = dose_matrix[orta_kesit_z, :, :]
dose_slice_masked = np.ma.masked_where(dose_slice < np.max(dose_slice)*0.05, dose_slice)
plt.imshow(dose_slice_masked, cmap='jet', alpha=0.7, interpolation='gaussian')

plt.colorbar(label='Proton Dozu (Bragg Peak)')
plt.title('Klinik Voksel Fantom: Akciger ve Tümör Üzerinde Proton Terapisi')
plt.xlabel('X Ekseni')
plt.ylabel('Y Ekseni')
plt.savefig('hasta_tedavi_plani.png', dpi=300)
print("Sahane! Gorsel 'hasta_tedavi_plani.png' olarak kaydedildi.")
