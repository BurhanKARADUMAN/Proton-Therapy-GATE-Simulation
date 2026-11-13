"""Command-line demonstration plus strict GATE .mhd + .raw analysis."""
import argparse
import csv
import json
from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt
from .physics import curve, StoppingModel
from .sobp import design_sobp
from .metrics import peak_metrics
from .metaimage import load_mhd, longitudinal_profile
from .plotting import save, style, COLORS, FG, MUTED


def _dump_csv(path, labels, rows):
    with Path(path).open('w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow(labels)
        writer.writerows(rows)


def make_demo(out: Path, histories: int = 3000) -> dict:
    out.mkdir(parents=True, exist_ok=True)
    d = np.linspace(0, 360, 721)
    energies = [70, 100, 130, 150, 180, 200]
    curves = [curve(e, d, histories=histories, seed=2026+i) for i, e in enumerate(energies)]
    fig, ax = plt.subplots(figsize=(10.5, 5.8))
    for e, c, color in zip(energies, curves, COLORS):
        ax.plot(d/10, c.normalized, lw=2, color=color, label=f"{e} MeV")
        ax.axvline(c.range_mm_nominal/10, ls=':', lw=.8, alpha=.2, color=color)
    style(ax)
    ax.set(xlabel="Depth in water (cm)", ylabel="Energy-loss proxy (per-curve normalized)",
           title="Bragg peak | Energy-dependent depth profiles")
    ax.legend(facecolor="#14203a", edgecolor="#3f516e", labelcolor=FG, ncol=3)
    ax.text(.01, -.19, "Illustrative residual-energy model · NOT GATE, measured dose or clinical Gy",
            color=MUTED, transform=ax.transAxes, fontsize=9)
    save(fig, out/'01_energy_sweep.png')

    materials = ['water','lung-like','bone-like']
    dm = np.linspace(0,650,1301)
    mc = [curve(150,dm,histories=histories,medium=x,seed=51) for x in materials]
    fig, ax = plt.subplots(figsize=(10.5,5.8))
    for medium, c, col in zip(materials,mc,COLORS[:3]):
        ax.plot(dm/10,c.normalized,label=f"{medium} (RSP demo)",color=col,lw=2.3)
    style(ax)
    ax.set(xlabel="Physical depth (cm)",ylabel="Energy-loss proxy (normalized)",
           title="Bragg-peak shift | Illustrative homogeneous medium substitutions")
    ax.legend(facecolor="#14203a", edgecolor="#3f516e", labelcolor=FG)
    ax.text(.01, -.19, "Demonstration RSPs are not calibrated CT-to-SPR values. No heterogeneous scattering modeled.",
            color=MUTED, transform=ax.transAxes,fontsize=9)
    save(fig, out/'02_medium_comparison.png')

    spreads = [0.0,0.005,0.02,0.05]
    profiles = [curve(150,d,histories=histories,energy_spread_fraction=s,seed=23) for s in spreads]
    fig,ax=plt.subplots(figsize=(10.5,5.8))
    for s,c,col in zip(spreads,profiles,COLORS[:4]):
        ax.plot(d/10,c.normalized,label=f"{100*s:.1f}% energy spread",color=col,lw=2.2)
    style(ax)
    ax.set(xlabel="Depth (cm)",ylabel="Energy-loss proxy (normalized)",
           title="Range uncertainty | Beam energy spread sensitivity")
    ax.legend(facecolor="#14203a", edgecolor="#3f516e", labelcolor=FG)
    ax.text(.01, -.19, "Finite sampled histories; simplified Gaussian toy beam with range straggling.",
            color=MUTED, transform=ax.transAxes,fontsize=9)
    save(fig, out/'03_uncertainty.png')

    # Broad energy layers produce peaks throughout the demonstration target.
    layers = np.arange(120, 154, 2.0)
    design = design_sobp(d,layers,target_start_mm=105,target_end_mm=155,
                         histories=max(200,histories//2),seed=77)
    fig=plt.figure(figsize=(10.5,8.3))
    grid=fig.add_gridspec(2,1,height_ratios=[3,1.4],hspace=.4)
    upper=fig.add_subplot(grid[0]); lower=fig.add_subplot(grid[1])
    for i in range(len(layers)):
        upper.plot(d/10,design.component_profiles[i]*design.weights[i],
                   color=COLORS[i%len(COLORS)],lw=1.1,alpha=.38)
    upper.plot(d/10,design.profile,color=COLORS[0],lw=3,label='Optimized superposition')
    upper.axvspan(10.5,15.5,color=COLORS[1],alpha=.12,label='Target interval')
    style(upper)
    upper.set(xlabel="Depth in water (cm)", ylabel="Energy-loss proxy (MeV/mm)",
              title="Spread-Out Bragg Peak | Optimization of monoenergetic layers")
    upper.legend(facecolor="#14203a",edgecolor="#3f516e",labelcolor=FG)
    lower.bar(np.arange(len(layers)),design.weights,width=.7,color=COLORS[2]);style(lower)
    lower.set_xticks(np.arange(len(layers)),labels=[str(int(v)) for v in layers])
    lower.set(xlabel="Proton energy layer (MeV)",ylabel="Nonnegative layer weight")
    save(fig,out/'04_sobp_optimization.png')

    _dump_csv(out/'energy_sweep.csv',['depth_mm']+[f'{x}_MeV_proxy' for x in energies],
              zip(d,*[c.energy_loss_mev_per_mm_primary for c in curves]))
    _dump_csv(out/'sobp_layers.csv',['energy_mev','optimized_weight'],zip(layers,design.weights))
    stats={"model":"illustrative analytic energy-loss proxy; NOT Gy or Geant4",
           "histories_per_energy_profile":histories,
           "range_fit":{"a_cm":StoppingModel().a_cm,"exponent":StoppingModel().exponent},
           "energy_sweep":{"energy_MeV":energies,
                           "peak_depth_mm":[peak_metrics(d,c.energy_loss_mev_per_mm_primary)['peak_depth_mm'] for c in curves]},
           "sobp":{"target_interval_mm":[105,155],"energy_layers_MeV":layers.tolist(),
                   "weights":design.weights.tolist(),
                   "rms_relative_nonuniformity_in_target":design.rms_relative_nonuniformity},
           "limitations":"No nuclear interactions, patient CT, lateral scatter, absolute dose, or clinical verification."}
    (out/'demo_metadata.json').write_text(json.dumps(stats,indent=2),encoding='utf-8')
    return stats


def make_gate_analysis(mhd_path: Path, output_dir: Path, axis: str, sign: int):
    img=load_mhd(mhd_path)
    depth, raw=longitudinal_profile(img,axis,sign)
    if np.any(~np.isfinite(raw)) or np.min(raw)<0:
        raise ValueError('Invalid dose image values')
    met=peak_metrics(depth,raw)
    output_dir.mkdir(parents=True,exist_ok=True)
    _dump_csv(output_dir/'gate_depth_profile.csv',['depth_mm','mean_voxel_value'],zip(depth,raw))
    fig,ax=plt.subplots(figsize=(10.3,5.5))
    ax.plot(depth,raw,color=COLORS[0],lw=2.5)
    style(ax)
    ax.set(title='GATE depth-profile extraction | Inspect input units and geometry',
           xlabel=f'Physical depth from {"+" if sign > 0 else "-"}{axis} entrance (mm)',
           ylabel='Mean of input voxel values (units inherited from GATE)')
    ax.text(.01,-.19,'Projection over complete transverse plane; verify beam axis, scoring bounds, and source alignment.',
            transform=ax.transAxes,color=MUTED,fontsize=9)
    save(fig,output_dir/'gate_depth_profile.png')
    (output_dir/'gate_metrics.json').write_text(json.dumps({"input":str(mhd_path),"axis":axis,"sign":sign,
         "spacing_mm":img.spacing_mm,"metrics":{k:(v if np.isfinite(v) else None) for k,v in met.items()},
         "warning":"Voxel outputs retain native GATE units; analysis does not assert Gy or patient dose."},indent=2),encoding='utf-8')


def main(argv=None):
    parser=argparse.ArgumentParser(description='Bragg Peak V2: transparent research/demo tool, NOT clinical dose prediction')
    parser.add_argument('--output-dir',default='examples/generated')
    parser.add_argument('--histories',type=int,default=3000)
    parser.add_argument('--mhd',type=Path,help='Analyze an existing GATE .mhd plus its external .raw')
    parser.add_argument('--axis',choices=['x','y','z'],default='x')
    parser.add_argument('--sign',type=int,choices=[-1,1],default=-1)
    args=parser.parse_args(argv)
    if args.mhd:
        make_gate_analysis(args.mhd,Path(args.output_dir),args.axis,args.sign)
        print('GATE depth-profile analysis saved to',args.output_dir)
    else:
        stats=make_demo(Path(args.output_dir),args.histories)
        print('Synthetic-only research illustrations generated:',args.output_dir)
        print('SOBP demo target relative RMS:',f"{stats['sobp']['rms_relative_nonuniformity_in_target']:.3f}")

if __name__=='__main__':
    main()
