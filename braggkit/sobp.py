"""Illustrative nonnegative optimisation of a Spread-Out Bragg Peak."""
from dataclasses import dataclass
import numpy as np
from scipy.optimize import lsq_linear
from .physics import curve


@dataclass(frozen=True)
class SOBP:
    energies_mev: np.ndarray
    weights: np.ndarray
    depth_mm: np.ndarray
    component_profiles: np.ndarray
    profile: np.ndarray
    target_start_mm: float
    target_end_mm: float
    rms_relative_nonuniformity: float


def design_sobp(
    depth_mm: np.ndarray,
    energies_mev: np.ndarray,
    *,
    target_start_mm: float,
    target_end_mm: float,
    histories: int = 2000,
    seed: int = 48,
    regularization: float = 0.01,
) -> SOBP:
    depth = np.asarray(depth_mm, dtype=float)
    energies = np.asarray(energies_mev, dtype=float)
    if energies.ndim != 1 or len(energies) < 3 or np.any(energies <= 0):
        raise ValueError("Provide >=3 positive energy layers")
    if target_start_mm >= target_end_mm or not (depth.min() < target_start_mm < target_end_mm < depth.max()):
        raise ValueError("The target must be inside the depth grid")
    if not 0 <= regularization < 1:
        raise ValueError("Regularization must be in [0,1)")
    comps = np.array([curve(float(e), depth, histories=histories, seed=seed + i).energy_loss_mev_per_mm_primary
                      for i, e in enumerate(energies)])
    target = (depth >= target_start_mm) & (depth <= target_end_mm)
    if target.sum() < 5:
        raise ValueError("Target region too narrow for grid")
    a = comps[:, target].T
    scale = np.max(a, axis=0)
    active = scale > 1e-9
    if not np.any(active):
        raise ValueError("No energy layer deposits energy in the target interval")
    normalized = a[:, active] / scale[active]
    reg = regularization * np.eye(int(active.sum()))
    system = np.vstack((normalized, reg))
    rhs = np.r_[np.ones(target.sum()), np.zeros(int(active.sum()))]
    fit = lsq_linear(system, rhs, bounds=(0, np.inf), tol=1e-9, max_iter=250)
    weights = np.zeros(len(energies))
    weights[active] = fit.x / scale[active]
    if not np.any(weights > 0):
        raise RuntimeError("The target optimizer produced no positive energy layers")
    weights /= weights.sum()
    mixture = weights @ comps
    within = mixture[target]
    rms = float(np.std(within) / np.mean(within))
    return SOBP(energies, weights, depth, comps, mixture,
                target_start_mm, target_end_mm, rms)
