import numpy as np
import pytest
from braggkit.sobp import design_sobp


def test_sobp_weights_nonnegative_and_reproducible():
    depth=np.linspace(0,280,561)
    energies=np.arange(115,161,5.)
    fit=design_sobp(depth,energies,target_start_mm=105,target_end_mm=160,histories=300)
    assert np.isclose(sum(fit.weights),1)
    assert np.all(fit.weights>=0)
    assert fit.profile.shape==depth.shape
    assert np.isfinite(fit.rms_relative_nonuniformity)
    fit2=design_sobp(depth,energies,target_start_mm=105,target_end_mm=160,histories=300)
    np.testing.assert_allclose(fit.weights,fit2.weights,rtol=0,atol=0)


def test_sobp_bad_inputs():
    with pytest.raises(ValueError): design_sobp(np.linspace(0,200,100), np.array([100,110]),target_start_mm=30,target_end_mm=90)
