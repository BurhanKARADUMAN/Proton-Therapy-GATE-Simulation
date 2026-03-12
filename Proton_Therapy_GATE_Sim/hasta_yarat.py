import numpy as np
import SimpleITK as sitk

print("Sanal Hasta CT'si Olusturuluyor...")
# 50x50x50 cm boyutlarinda bir hasta (1 voksel = 1 cm)
# 0 = Hava (Akciger), 1 = Su (Yumusak Doku), 2 = Kemik
matrix = np.zeros((50, 50, 50), dtype=np.uint16)

# Vucudu Su (1) ile doldur
for z in range(50):
    for y in range(50):
        for x in range(50):
            if (x-25)**2 + (y-25)**2 < 20**2:
                matrix[z, y, x] = 1

# Icine iki tane Akciger boslugu (Hava - 0) acalim
for z in range(10, 40):
    for y in range(50):
        for x in range(50):
            if (x-15)**2 + (y-20)**2 < 6**2: # Sol akciger
                matrix[z, y, x] = 0
            if (x-35)**2 + (y-20)**2 < 6**2: # Sag akciger
                matrix[z, y, x] = 0

# Omurga (2) ekleyelim
for z in range(5, 45):
    for y in range(50):
        for x in range(50):
            if (x-25)**2 + (y-35)**2 < 3**2:
                matrix[z, y, x] = 2

img = sitk.GetImageFromArray(matrix)
img.SetSpacing((10.0, 10.0, 10.0)) # Voksel boyutu 10mm
sitk.WriteImage(img, 'hasta_bt.mhd')
print("Sanal Hasta (hasta_bt.mhd) basariyla uretildi!")
