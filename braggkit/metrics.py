"""Peak and distal-falloff measurements, explicit crossing definitions."""
from typing import Mapping
import numpy as np


def _distal_crossing(depth, y, fraction):
    peak = int(np.argmax(y))
    limit = float(y[peak]) * fraction
    # Locate first crossing AFTER the peak. If no crossing exists, return NaN.
    for j in range(peak + 1, y.size):
        if y[j] <= limit and y[j-1] >= limit:
            x0, x1 = float(depth[j-1]), float(depth[j])
            y0, y1 = float(y[j-1]), float(y[j])
            return x0 if y0 == y1 else x0 + (limit-y0) * (x1-x0) / (y1-y0)
    return float("nan")


def peak_metrics(depth_mm: np.ndarray, values: np.ndarray) -> Mapping[str, float]:
    depth = np.asarray(depth_mm, dtype=float)
    y = np.asarray(values, dtype=float)
    if depth.ndim != 1 or depth.size < 3 or depth.shape != y.shape:
        raise ValueError("Require matched 1D depth and signal arrays")
    if np.any(np.diff(depth) <= 0) or np.any(~np.isfinite(y)) or np.any(y < 0):
        raise ValueError("Invalid depth or non-negative profile")
    i = int(np.argmax(y))
    if y[i] <= 0:
        raise ValueError("Profile is identically zero")
    r90 = _distal_crossing(depth, y, .9)
    r80 = _distal_crossing(depth, y, .8)
    r20 = _distal_crossing(depth, y, .2)
    entrance = float(np.mean(y[:min(10, len(y))]))
    return {
        "peak_depth_mm": float(depth[i]),
        "peak_height": float(y[i]),
        "entrance_to_peak_ratio": float(entrance / y[i]),
        "distal_R90_mm": r90,
        "distal_R80_mm": r80,
        "distal_R20_mm": r20,
        "distal_80_to_20_mm": float(r20-r80),
    }
