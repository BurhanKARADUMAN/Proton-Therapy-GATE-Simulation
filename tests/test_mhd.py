from pathlib import Path
import numpy as np
import pytest
from braggkit.metaimage import load_mhd, longitudinal_profile


def make_fixture(tmp_path, *, dims=(6,5,4), data=None, element='MET_FLOAT'):
    data=np.arange(np.prod(dims),dtype=np.float32).reshape(dims[::-1]) if data is None else data
    raw=tmp_path/'dose.raw'
    data.astype('<f4').tofile(raw)
    header=tmp_path/'dose.mhd'
    header.write_text(f'ObjectType = Image\nNDims = 3\nBinaryData = True\nCompressedData = False\n'
                      f'DimSize = {dims[0]} {dims[1]} {dims[2]}\n'
                      f'ElementType = {element}\nElementSpacing = 1 2 3\n'
                      'Offset = -5 -4 -3\nElementDataFile = dose.raw\n')
    return header


def test_mhd_roundtrip_and_axis_sign(tmp_path):
    hdr=make_fixture(tmp_path)
    image=load_mhd(hdr)
    assert image.volume.shape==(4,5,6)
    assert image.spacing_mm==(1.0,2.0,3.0)
    d,y=longitudinal_profile(image,'x',-1)
    assert len(d)==6 and np.isclose(d[-1],5.0)
    np.testing.assert_allclose(y,np.mean(image.volume,axis=(0,1))[::-1])
    dz,z=longitudinal_profile(image,'z',1,projection='sum')
    assert len(dz)==4 and dz[-1]==9
    assert z[0]==image.volume[0].sum()


def test_missing_raw_fails_loudly(tmp_path):
    hdr=make_fixture(tmp_path)
    (tmp_path/'dose.raw').unlink()
    with pytest.raises(FileNotFoundError, match='Missing .raw payload'):
        load_mhd(hdr)


def test_size_mismatch_rejected(tmp_path):
    hdr=make_fixture(tmp_path)
    (tmp_path/'dose.raw').write_bytes(b'abc')
    with pytest.raises(ValueError,match='Invalid raw size'):load_mhd(hdr)


def test_compressed_not_supported(tmp_path):
    hdr=make_fixture(tmp_path)
    s=hdr.read_text().replace('CompressedData = False','CompressedData = True')
    hdr.write_text(s)
    with pytest.raises(ValueError,match='Compressed'):load_mhd(hdr)


def test_bad_axis_rejected(tmp_path):
    im=load_mhd(make_fixture(tmp_path))
    with pytest.raises(ValueError):longitudinal_profile(im,'q')
    with pytest.raises(ValueError):longitudinal_profile(im,'x',3)


def test_missing_header(tmp_path):
    with pytest.raises(FileNotFoundError):load_mhd(tmp_path/'missing.mhd')
