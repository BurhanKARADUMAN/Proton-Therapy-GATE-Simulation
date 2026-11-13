"""Explicit simplified 1D proton range / Bragg-peak demonstration.

NOT a transport calculation or absorbed-dose-to-water model. No scattering,
secondaries, nonelastic nuclear interactions, heterogeneous 3D transport,
or experimentally validated range-energy calibration are included.
"""
from dataclasses import dataclass
from typing import Union
import numpy as np

# Rough, illustrative water-equivalent range fit. Do NOT use clinically.
DEFAULT_A_CM = 0.0022
DEFAULT_EXPONENT = 1.77
# Demonstration ratios only; not CT-calibrated RSP values.
RSP_REFERENCE = {"water": 1.0, "lung-like": 0.30, "bone-like": 1.60}


@dataclass(frozen=True)
class StoppingModel:
    a_cm: float = DEFAULT_A_CM
    exponent: float = DEFAULT_EXPONENT

    def __post_init__(self):
        if self.a_cm <= 0 or self.exponent <= 1:
            raise ValueError("a_cm must be positive and exponent > 1")

    def range_mm(self, energy_mev: Union[float, np.ndarray]):
        e = np.asarray(energy_mev, dtype=float)
        if not np.all(np.isfinite(e)) or np.any(e <= 0):
            raise ValueError("Energy must be finite and > 0")
        return 10.0 * self.a_cm * e ** self.exponent


def medium_rspr(medium: str) -> float:
    try:
        return RSP_REFERENCE[medium]
    except KeyError as err:
        raise ValueError("Unknown demonstration medium: " + str(medium)) from err


@dataclass(frozen=True)
class DepthDose:
    depth_mm: np.ndarray
    energy_loss_mev_per_mm_primary: np.ndarray
    range_mm_nominal: float
    energy_mev: float
    histories: int
    medium: str
    integral_mev_per_primary: float
    standard_error_mev_per_mm: np.ndarray

    @property
    def normalized(self) -> np.ndarray:
        m = float(np.max(self.energy_loss_mev_per_mm_primary))
        return self.energy_loss_mev_per_mm_primary / m if m else np.zeros_like(self.depth_mm)


def curve(
    energy_mev: float,
    depth_mm: np.ndarray,
    *,
    histories: int = 4000,
    energy_spread_fraction: float = 0.005,
    range_straggling_fraction: float = 0.012,
    medium: str = "water",
    seed: int = 2026,
    model: StoppingModel | None = None,
    rsp_override: float | None = None,
    batch_size: int = 512,
) -> DepthDose:
    """Return per-primary mean deposited-energy proxy (MeV/mm).

    Each primary has a sampled initial energy and a normally perturbed
    water-equivalent range.  Deposited energy in each bin is the difference
    of a residual-energy function evaluated at consecutive bin edges:
      E_res(W) = E0 * max(1 - W / R_sample, 0)**(1 / p)
    where W = z * RSP.  This construction conserves each primary's energy
    when the scoring domain spans its stop, but omits nuclear escape losses.
    SE estimates the toy-sample Monte Carlo variability, not physics-model
    systematic uncertainty. Results are not Gy and not measured dose.
    """
    depth = np.asarray(depth_mm, dtype=float)
    if depth.ndim != 1 or depth.size < 4 or not np.all(np.isfinite(depth)):
        raise ValueError("depth_mm must be a finite one-dimensional grid")
    if np.any(np.diff(depth) <= 0) or depth[0] < 0:
        raise ValueError("Depths must strictly increase from >=0")
    if not np.isfinite(energy_mev) or energy_mev <= 0:
        raise ValueError("energy_mev must be positive")
    if histories < 2 or batch_size < 1:
        raise ValueError("histories >= 2 and batch_size >= 1 are required")
    if (not 0 <= energy_spread_fraction < 0.5 or
            not 0 <= range_straggling_fraction < 0.5):
        raise ValueError("Fractions must lie in [0, 0.5)")
    rsp = medium_rspr(medium) if rsp_override is None else float(rsp_override)
    if not np.isfinite(rsp) or rsp <= 0:
        raise ValueError("Relative stopping power must be positive")
    model = model or StoppingModel()
    rng = np.random.default_rng(seed)
    energies = np.maximum(rng.normal(energy_mev, energy_mev * energy_spread_fraction, histories), 0.001)
    nominal_ranges = model.range_mm(energies)
    ranges = np.maximum(rng.normal(nominal_ranges, nominal_ranges * range_straggling_fraction), 0.001)
    # Sample at the depth-bin edges, preserving energy exactly by differencing.
    edges = np.empty(depth.size + 1)
    edges[1:-1] = (depth[:-1] + depth[1:]) / 2
    edges[0] = max(0.0, depth[0] - (depth[1] - depth[0]) / 2)
    edges[-1] = depth[-1] + (depth[-1] - depth[-2]) / 2
    widths = np.diff(edges)
    wet = edges * rsp
    first = np.zeros(depth.size)
    second = np.zeros(depth.size)
    for start in range(0, histories, batch_size):
        en = energies[start:start + batch_size, None]
        rg = ranges[start:start + batch_size, None]
        remaining = en * np.maximum(1 - wet[None, :] / rg, 0) ** (1 / model.exponent)
        bin_loss = np.maximum(0, remaining[:, :-1] - remaining[:, 1:]) / widths
        first += bin_loss.sum(axis=0)
        second += np.square(bin_loss).sum(axis=0)
    mean = first / histories
    variance = np.maximum(second / histories - mean**2, 0.0)
    se = np.sqrt(variance / (histories - 1))
    return DepthDose(
        depth_mm=depth,
        energy_loss_mev_per_mm_primary=mean,
        range_mm_nominal=float(model.range_mm(energy_mev)) / rsp,
        energy_mev=energy_mev,
        histories=histories,
        medium=medium,
        integral_mev_per_primary=float(np.sum(mean * widths)),
        standard_error_mev_per_mm=se,
    )
