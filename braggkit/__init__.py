"""Transparent, non-clinical proton depth-dose research tools."""
from .physics import StoppingModel, DepthDose, curve, medium_rspr
from .metrics import peak_metrics
__version__ = "2.0.0"
__all__ = ["StoppingModel", "DepthDose", "curve", "medium_rspr", "peak_metrics"]
