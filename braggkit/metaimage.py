"""Dependency-light reader of uncompressed external-buffer 3D MetaImage files.

Data shape is (z, y, x), matching the usual ITK / SimpleITK numpy convention.
This utility does NOT resample, register or convert absorbed-dose units.
"""
from dataclasses import dataclass
from pathlib import Path
import numpy as np

TYPES = {
    "MET_FLOAT": "f4", "MET_DOUBLE": "f8", "MET_SHORT": "i2",
    "MET_USHORT": "u2", "MET_CHAR": "i1", "MET_UCHAR": "u1", "MET_INT": "i4",
    "MET_UINT": "u4"
}


@dataclass(frozen=True)
class MetaImage:
    volume: np.ndarray
    spacing_mm: tuple[float, float, float]
    origin_mm: tuple[float, float, float]
    header_path: Path


def load_mhd(path: str | Path) -> MetaImage:
    header = Path(path).expanduser().resolve()
    if not header.is_file():
        raise FileNotFoundError(f"MetaImage header missing: {header}")
    lines = header.read_text(encoding="utf-8").splitlines()
    props = {}
    for line in lines:
        if "=" in line and not line.strip().startswith("#"):
            k, val = line.split("=", 1)
            props[k.strip()] = val.strip()
    if props.get("NDims") != "3":
        raise ValueError("Only 3D MetaImage is supported")
    if props.get("CompressedData", "False").lower() == "true":
        raise ValueError("Compressed .mhd data not supported")
    if props.get("ElementDataFile", "LOCAL").upper() == "LOCAL":
        raise ValueError("Inline LOCAL data not supported; use external .raw")
    if ".." in Path(props["ElementDataFile"]).parts or Path(props["ElementDataFile"]).is_absolute():
        raise ValueError("External .raw must be next to header, without path traversal")
    raw = header.parent / props["ElementDataFile"]
    if not raw.is_file():
        raise FileNotFoundError(f"Missing .raw payload: {raw} (a .mhd header alone is not dose data)")
    dims = tuple(map(int, props["DimSize"].split()))
    if len(dims) != 3 or min(dims) < 1:
        raise ValueError("Invalid DimSize")
    spacings = tuple(map(float, props["ElementSpacing"].split()))
    if len(spacings) != 3 or not all(np.isfinite(spacings)) or min(spacings) <= 0:
        raise ValueError("Invalid ElementSpacing")
    offset = tuple(map(float, props.get("Offset", "0 0 0").split()))
    dt = TYPES.get(props["ElementType"])
    if dt is None:
        raise ValueError(f"Unsupported ElementType {props['ElementType']}")
    endian = ">" if props.get("BinaryDataByteOrderMSB", "False").lower() == "true" else "<"
    dtype = np.dtype(endian + dt)
    expected = int(np.prod(dims)) * dtype.itemsize
    if raw.stat().st_size != expected:
        raise ValueError(f"Invalid raw size: {raw.stat().st_size} B, expected {expected} B")
    array = np.fromfile(raw, dtype=dtype).reshape(dims[::-1]).astype(float, copy=False)
    return MetaImage(array, spacings, offset, header)


def longitudinal_profile(img: MetaImage, beam_axis: str = "x", beam_sign: int = -1,
                         projection: str = "mean") -> tuple[np.ndarray, np.ndarray]:
    """Collapse transverse plane into depth axis in beam propagation direction.

    beam_axis refers to the physical ITK x/y/z coordinate.  Depth zero is
    the first voxel in the specified beam travel direction. The output retains
    the raw intensity units, with NO Gy / MeV conversion or ROI alignment.
    """
    if beam_axis not in {"x", "y", "z"} or beam_sign not in {-1, 1}:
        raise ValueError("beam_axis must be x/y/z and beam_sign +/-1")
    if projection not in {"mean", "sum"}:
        raise ValueError("projection must be mean or sum")
    array_axis = {"x": 2, "y": 1, "z": 0}[beam_axis]
    other = tuple(i for i in range(3) if i != array_axis)
    values = getattr(np, projection)(img.volume, axis=other)
    if beam_sign < 0:
        values = values[::-1]
    step = img.spacing_mm[{"x": 0, "y": 1, "z": 2}[beam_axis]]
    depth = np.arange(values.size, dtype=float) * step
    return depth, np.asarray(values)
