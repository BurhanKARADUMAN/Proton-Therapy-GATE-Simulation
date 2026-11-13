import numpy as np
import pytest
from braggkit.physics import StoppingModel, curve, medium_rspr
from braggkit.metrics import peak_metrics

D = np.linspace(0, 480, 961)


def test_range_increases_with_energy():
    m=StoppingModel()
    assert m.range_mm(80)<m.range_mm(150)<m.range_mm(220)


def test_range_is_positive_and_finite():
    a=StoppingModel().range_mm(np.array([50.,100.,150.,200.]))
    assert np.all(np.isfinite(a)) and np.all(a>0)


def test_model_rejects_bad_parameters():
    with pytest.raises(ValueError): StoppingModel(a_cm=-1)
    with pytest.raises(ValueError): StoppingModel(exponent=0.5)
    with pytest.raises(ValueError): StoppingModel().range_mm(-2)


def test_zero_spread_energy_conservation():
    # The 1D toy model exactly conserves initial energy when the depth interval
    # fully covers each stop (this is NOT full physical proton transport).
    c=curve(150,D,histories=300,energy_spread_fraction=0,
            range_straggling_fraction=0)
    assert np.isclose(c.integral_mev_per_primary,150,rtol=1e-8)


def test_reproducible_seed():
    a=curve(150,D,histories=300,seed=31)
    b=curve(150,D,histories=300,seed=31)
    np.testing.assert_array_equal(a.energy_loss_mev_per_mm_primary,
                                  b.energy_loss_mev_per_mm_primary)


def test_peak_moves_with_energy():
    a=curve(100,D,histories=700,seed=2)
    b=curve(180,D,histories=700,seed=2)
    assert peak_metrics(D,a.energy_loss_mev_per_mm_primary)['peak_depth_mm'] < peak_metrics(D,b.energy_loss_mev_per_mm_primary)['peak_depth_mm']


def test_bragg_peak_is_above_entrance():
    a=curve(150,D,histories=800,seed=2)
    stats=peak_metrics(D,a.energy_loss_mev_per_mm_primary)
    assert 0<stats['entrance_to_peak_ratio']<0.9


def test_medium_range_ordering():
    lung=curve(150,D,histories=200,medium='lung-like')
    water=curve(150,D,histories=200,medium='water')
    bone=curve(150,D,histories=200,medium='bone-like')
    assert bone.range_mm_nominal < water.range_mm_nominal < lung.range_mm_nominal


def test_nonnegative_variability():
    a=curve(130,D,histories=1000)
    assert np.min(a.energy_loss_mev_per_mm_primary)>=0
    assert np.min(a.standard_error_mev_per_mm)>=0


@pytest.mark.parametrize('kwargs',[
    {'histories':1}, {'energy_spread_fraction':0.6}, {'range_straggling_fraction':-0.01},
    {'medium':'unobtanium'}, {'rsp_override':0}, {'batch_size':0}])
def test_curve_invalid_arguments(kwargs):
    with pytest.raises(ValueError): curve(120,D,**kwargs)


def test_invalid_depth_grid():
    with pytest.raises(ValueError): curve(150,np.array([0.,2.,2.,3.]))


def test_distal_metrics_defined_for_full_window():
    a=curve(140,D,histories=900)
    m=peak_metrics(D,a.energy_loss_mev_per_mm_primary)
    assert m['distal_R90_mm'] < m['distal_R80_mm'] < m['distal_R20_mm']


def test_medium_keys():
    assert medium_rspr('water')==1
    with pytest.raises(ValueError): medium_rspr('steel')


def test_negative_energy():
    with pytest.raises(ValueError):curve(-10,D)
