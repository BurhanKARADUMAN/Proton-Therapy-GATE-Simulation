# Windows'ta mevcut GitHub deposuna yeni sürümü ekleme

**En kolay yöntem:** Bu ZIP'i çıkarıp `BraggPeak_V2_Upgrade` klasörünün adres çubuğundan `powershell` açın ve `powershell -ExecutionPolicy Bypass -File .\APPLY_TO_GITHUB.ps1` komutunu çalıştırın. Script yalnızca hedef proton terapisi deposunun **yeni bir klonunda** çalışır; eskileri silmez ve force-push yapmaz. İsterseniz aşağıdaki manuel yöntemi kullanın.

Bu ZIP bir **ek/güncelleme paketidir**: mevcut GATE dosyalarını içermez ve silmez. `Proton_Therapy_GATE_Sim` klasörünün olduğu mevcut GitHub çalışma klasörünün üstüne yeni dosyaları kopyalayın. `README.md` dosyasını yeni İngilizce sürümle değiştirir. Eski ayrıntılı rapor `Proton_Therapy_GATE_Sim/README.md.txt` içinde kalır.

1. İndirilen `BraggPeak_V2_Upgrade.zip` dosyasını çıkarın.
2. Çıkan `BraggPeak_V2_Upgrade` klasöründeki **tüm içeriği** GitHub'daki `Proton-Therapy-GATE-Simulation` klasörünün köküne kopyalayın. `.github` gizli klasörünü de kopyaladığınızdan emin olun.
3. Bu kök klasörde PowerShell açın.
4. Python kurulumu/testleri (Python 3.10+):

```powershell
py -m pip install -e ".[test]"
py -m pytest -q
py -m braggkit.cli --output-dir examples\generated --histories 3000
```

5. GitHub'a yayımlayın:

```powershell
git status
git add README.md braggkit app.py tests docs gate examples .github pyproject.toml .gitignore CHANGELOG_V2.md HOW_TO_MERGE_WINDOWS_TR.md
git commit -m "feat: Bragg Peak Lab V2 research toolkit"
git push origin main
```

İlk kez klonluyorsanız, Github repo sayfasındaki **Code → Download ZIP** ile indirilen arşivde `.git` klasörü olmayabilir. Git ile eşleştirmek için `git clone https://github.com/BurhanKARADUMAN/Proton-Therapy-GATE-Simulation.git` kullanın ve yukarıdaki ZIP içeriğini klonlanan klasöre taşıyın.

**Not:** Geçmiş `.raw` dosyaları gizli kalmış olabilir. Yeni script gerçek GATE analizi için `.mhd` ile aynı klasörde doğru `.raw` dosyasını gerektirir.
