"""Publication-conscious figures with correct units and caveats."""
from pathlib import Path
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator

BG = "#0d1528"
PANEL = "#14203a"
FG = "#e6efff"
MUTED = "#9eb0cc"
COLORS = ["#69e0c3", "#fdbf69", "#81aaff", "#d4a4ff", "#ff7a9b", "#7bd6ee"]


def style(ax):
    ax.set_facecolor(PANEL)
    ax.tick_params(colors=MUTED, labelsize=9)
    for k in ax.spines:
        ax.spines[k].set_color("#3f516e")
    ax.xaxis.label.set_color(FG)
    ax.yaxis.label.set_color(FG)
    ax.title.set_color(FG)
    ax.grid(alpha=.13, color=FG, lw=.7)
    ax.xaxis.set_minor_locator(AutoMinorLocator())
    return ax


def save(fig, path):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    fig.set_facecolor(BG)
    fig.savefig(path, dpi=170, facecolor=BG, bbox_inches="tight")
    plt.close(fig)
    return path
