"""Optional interactive Streamlit app: streamlit run app.py"""
import numpy as np
import matplotlib.pyplot as plt
import streamlit as st
from braggkit.physics import curve, StoppingModel
from braggkit.metrics import peak_metrics

st.set_page_config(page_title='Bragg Peak Lab', page_icon='⚛️', layout='wide')
st.title('⚛️ Bragg Peak Lab')
st.caption('Transparent, educational proton energy-loss proxy • Not a GATE calculation, measured dose, or treatment-planning system')
with st.sidebar:
    st.header('Proton beam')
    energy=st.slider('Nominal energy (MeV)',50,250,150,5)
    medium=st.selectbox('Homogeneous medium', ['water','lung-like','bone-like'])
    spread=st.slider('Energy spread (%)',0.0,6.0,0.5,0.1)
    straggle=st.slider('Range straggling (%)',0.0,6.0,1.2,0.1)
    histories=st.slider('Toy histories',200,5000,1400,200)
    st.caption('Medium ratios are illustrative, NOT patient CT calibrated.')
depth=np.linspace(0,650,651)
c=curve(energy,depth,medium=medium,histories=histories,
        energy_spread_fraction=spread/100,range_straggling_fraction=straggle/100)
m=peak_metrics(depth,c.energy_loss_mev_per_mm_primary)
a,b,c_metric=st.columns(3)
a.metric('Nominal physical range',f'{c.range_mm_nominal/10:.1f} cm')
b.metric('Peak position',f'{m["peak_depth_mm"]/10:.1f} cm')
c_metric.metric('Distal 80–20% falloff',f'{m["distal_80_to_20_mm"]:.1f} mm')
fig,ax=plt.subplots(figsize=(11,4.8))
ax.plot(depth/10,c.normalized,lw=2.5,color='#22b8a2')
ax.fill_between(depth/10,c.normalized,alpha=.12,color='#22b8a2')
ax.axvline(c.range_mm_nominal/10,color='#d89d2f',ls='--',label='Nominal range')
ax.set(xlabel='Physical depth (cm)',ylabel='Energy-loss proxy (normalized)',title=f'{energy} MeV in {medium}')
ax.grid(alpha=.18);ax.legend();st.pyplot(fig,clear_figure=True)
st.info('The stopping-range power law is approximate and omits nuclear interactions, 3D transport, scatter, RBE, and dosimetry calibration. Never use for patients.')
