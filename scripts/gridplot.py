import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import matplotlib.colors as mcolors
import numpy as np
import operator

# ── helpers ────────────────────────────────────────────────────────────────

_OPS = {
    ">": operator.gt,
    ">=": operator.ge,
    "<": operator.lt,
    "<=": operator.le,
}

_HATCHES = ["/", "\\", "x", "+", "-", "|", "//", "\\\\"]
_COLORS = [
    "#4e9af1",  "#f4c542", "#6bcb77",
    "#c77dff", "#ff9f1c", "#2ec4b6", "#e71d36", "#f97b6b",
]


def ruleset_to_regions(ruleset):
    return [rule_to_region(rule) for rule in ruleset]


def rule_to_region(rule):
    region = []
    for clause in rule:
        feature, val = clause.split(">=")
        val = float(val)
        op = ">="
        if feature.startswith("-"):
            feature = feature[1:]
            val = -val
            op = "<="
        region.append((feature, val, op))
    return region


def _region_bounds(conditions, col1, col2, x1_lim, x2_lim):
    x1_min, x1_max = float(x1_lim[0]), float(x1_lim[1])
    x2_min, x2_max = float(x2_lim[0]), float(x2_lim[1])

    for (var, thresh, op_str) in conditions:
        op_str = op_str.strip()
        if var == col1:
            if op_str in (">", ">="):
                x1_min = max(x1_min, thresh)
            elif op_str in ("<", "<="):
                x1_max = min(x1_max, thresh)
        elif var == col2:
            if op_str in (">", ">="):
                x2_min = max(x2_min, thresh)
            elif op_str in ("<", "<="):
                x2_max = min(x2_max, thresh)
        else:
            raise ValueError(f"Unknown variable '{var}'. Expected '{col1}' or '{col2}'.")

    if x1_min >= x1_max or x2_min >= x2_max:
        return None

    return x1_min, x2_min, x1_max - x1_min, x2_max - x2_min


def _fmt_sci(v):
    if v != 0 and (abs(v) < 0.01 or abs(v) >= 1e4):
        exp = int(np.floor(np.log10(abs(v))))
        return f"10^{{{exp}}}"
    return f"{v:.4g}"


def _region_label(conditions, feature_map):
    parts = [(f"{feature_map[var]} "
              f"${op.replace('>=', '\\geq').replace('<=', '\\leq')} "
              f"{thresh:.2f}$") for var, thresh, op in conditions]
    return " ∧ \n ".join(parts)


def _draw_margin(ax, conditions, col1, col2, x0, y0, w, h,
                 margin_width, color, x1_lim, x2_lim):
    x1_min_m, x2_min_m = x0, y0
    x1_max_m, x2_max_m = x0 + w, y0 + h

    for var, thresh, op in conditions:
        if var == col1:
            if op in (">", ">="):
                x1_min_m = thresh - margin_width
            else:
                x1_max_m = thresh + margin_width
        elif var == col2:
            if op in (">", ">="):
                x2_min_m = thresh - margin_width
            else:
                x2_max_m = thresh + margin_width

    x1_min_m = max(x1_min_m, x1_lim[0])
    x2_min_m = max(x2_min_m, x2_lim[0])
    x1_max_m = min(x1_max_m, x1_lim[1])
    x2_max_m = min(x2_max_m, x2_lim[1])

    margin_rect = mpatches.Rectangle(
        (x1_min_m, x2_min_m),
        x1_max_m - x1_min_m, x2_max_m - x2_min_m,
        facecolor=color, alpha=0.15,
        linewidth=1.0, edgecolor=color,
        linestyle=":", zorder=1
    )
    ax.add_patch(margin_rect)


def plot_2d_grid(X,
                 y,
                 x1_lim: tuple[float, float] | None = None,
                 x2_lim: tuple[float, float] | None = None,
                 regions=None,
                 n_gridlines: tuple[int, int] = (4, 4),
                 title: str = "2-D Feature Space",
                 cloud_centers=None,
                 feature_labels=("$x_1$", "$x_2$"),
                 margin_width: float = 0.0,
                 fig_w: float = 9.0,
                 fig_h: float = 6.0,
                 left: float = 0.8,
                 right: float = 2.4,
                 bottom: float = 0.6,
                 top: float = 0.5,
                 suptitle: str = None
                 ):
    """
    Plot a 2D grid with scatter points and optional shaded threshold regions.

    Design goals:
    - Fixed total figure size (fig_w x fig_h), regardless of title/legend content.
    - No stretching: the axes box's width:height ratio always matches the
      data's x1_range:x2_range ratio exactly.
    - Consistent placement: the axes box's top-left corner is anchored at the
      same point (left, fig_h - top) in every figure, so plots line up
      cleanly when arranged in a grid, even if aspect ratios happen to differ
      (only the bottom/right edge would shift in that case).

    Parameters
    ----------
    X           : pd.DataFrame with exactly 2 columns
    y           : array-like of bool/int labels
    x1_lim      : (min, max) for x-axis; inferred from data if None
    x2_lim      : (min, max) for y-axis; inferred from data if None
    regions     : list of condition lists [(var, thresh, op), ...]
    n_gridlines : (n1, n2) quantile gridlines per axis
    title       : plot title
    cloud_centers : list of (x, y) tuples
    feature_labels : (label1, label2) axis labels
    margin_width : width of margin band around regions
    fig_w, fig_h : total figure size in inches (constant across all plots)
    left, right, bottom, top : margins in inches, reserved space around the
                                axes box for labels/title/legend
    """
    col1, col2 = X.columns
    x1 = X[col1].to_numpy(dtype=float)
    x2 = X[col2].to_numpy(dtype=float)

    if x1_lim is None:
        pad = (x1.max() - x1.min()) * 0.05
        x1_lim = (x1.min() - pad, x1.max() + pad)
    if x2_lim is None:
        pad = (x2.max() - x2.min()) * 0.05
        x2_lim = (x2.min() - pad, x2.max() + pad)

    y = np.asarray(y, dtype=bool)
    n1, n2 = n_gridlines

    pos_mask = y
    neg_mask = ~y

    x1_range = x1_lim[1] - x1_lim[0]
    x2_range = x2_lim[1] - x2_lim[0]
    aspect = x2_range / x1_range  # height / width, in data units

    avail_w = fig_w - left - right
    avail_h = fig_h - top - bottom

    # largest axes box with correct aspect that fits in the available area
    if avail_h / avail_w > aspect:
        ax_w = avail_w
        ax_h = avail_w * aspect
    else:
        ax_h = avail_h
        ax_w = avail_h / aspect

    # anchor the axes box's top-left corner at a fixed point in every figure
    ax_left = left
    ax_bottom = (fig_h - top) - ax_h

    fig = plt.figure(figsize=(fig_w, fig_h))
    ax = fig.add_axes([
        ax_left   / fig_w,
        ax_bottom / fig_h,
        ax_w      / fig_w,
        ax_h      / fig_h,
    ])

    # ── shaded regions ─────────────────────────────────────────────────────
    feature_map = {col1: feature_labels[0], col2: feature_labels[1]}
    region_handles = []
    for i, conditions in enumerate(regions or []):
        color = _COLORS[i % len(_COLORS)]
        hatch = _HATCHES[i % len(_HATCHES)]
        bounds = _region_bounds(conditions, col1, col2, x1_lim, x2_lim)

        if bounds is None:
            continue

        x0, y0, w, h = bounds
        rect = mpatches.Rectangle(
            (x0, y0), w, h,
            linewidth=2.0,
            edgecolor=color,
            facecolor=color,
            alpha=0.5,
            hatch=hatch,
            zorder=2,
        )
        ax.add_patch(rect)

        proxy = mpatches.Patch(
            facecolor=color,
            edgecolor=color,
            alpha=0.6,
            hatch=hatch,
            label=_region_label(conditions, feature_map),
        )
        region_handles.append(proxy)

        if margin_width > 0:
            _draw_margin(ax, conditions, col1, col2, x0, y0, w, h,
                         margin_width, color, x1_lim, x2_lim)

    # ── positive points ────────────────────────────────────────────────────
    scatter_pos = ax.scatter(
        x1[pos_mask], x2[pos_mask],
        marker="o",
        facecolors="black",
        edgecolors="black",
        s=30,
        linewidths=1.0,
        label="Positive ($y = 1$)",
        zorder=3,
    )

    # ── negative points ────────────────────────────────────────────────────
    scatter_neg = ax.scatter(
        x1[neg_mask], x2[neg_mask],
        marker="o",
        facecolors="none",
        edgecolors="darkred",
        s=30,
        linewidths=1.0,
        label="Negative ($y = 0$)",
        zorder=3,
    )

    # ── centers ────────────────────────────────────────────────────────────
    if cloud_centers is not None:
        cx = [c[0] for c in cloud_centers]
        cy = [c[1] for c in cloud_centers]
        ax.scatter(cx, cy, marker="*", facecolors="steelblue", edgecolors="steelblue",
                   s=200, linewidths=1.0, zorder=4)

    ax.set_xlim(x1_lim)
    ax.set_ylim(x2_lim)

    # ── quantile grid lines ────────────────────────────────────────────────
    ax.set_axisbelow(True)
    ax.grid(False)
    ax.spines[["top", "right"]].set_visible(False)

    for k in range(1, n1 + 1):
        q = np.quantile(x1, k / (n1 + 1))
        ax.plot([q, q], [x2_lim[0], x2_lim[1]], color="gray", linewidth=1.0,
                alpha=0.4, zorder=0, linestyle=":")
    for k in range(1, n2 + 1):
        q = np.quantile(x2, k / (n2 + 1))
        ax.plot([x1_lim[0], x1_lim[1]], [q, q], color="gray", linewidth=1.0,
                alpha=0.4, zorder=0, linestyle=":")

    flabel1, flabel2 = feature_labels
    ax.set_xlabel(flabel1, fontsize=16)
    ax.set_ylabel(flabel2, fontsize=16)
    if suptitle is None:
        ax.set_title(title, fontsize=20, pad=12)
    else:
        fig.suptitle(suptitle, fontsize=20)
        ax.set_title(title, fontsize=15)

    # ── legend ─────────────────────────────────────────────────────────────
    grid_handle = ax.plot([], [], color="gray", linewidth=1.0, alpha=0.6,
                          linestyle=":", label="Thresholds")[0]

    center_handles = (
        [ax.scatter([], [], marker="*", facecolors="steelblue",
                    edgecolors="steelblue", s=200, label="Cloud centers")]
        if cloud_centers else []
    )

    ax.legend(
        handles=[scatter_pos, scatter_neg] + center_handles + region_handles + [grid_handle],
        frameon=True,
        fontsize=13,
        loc="upper left",
        bbox_to_anchor=(1.02, 1),
        borderaxespad=0,
    )

    return fig, ax


# ── quick demo ─────────────────────────────────────────────────────────────
if __name__ == "__main__":
    import pandas as pd

    rng = np.random.default_rng(42)
    n = 60

    X_demo = pd.DataFrame({
        "age":    rng.uniform(-3, 3, n),
        "income": rng.uniform(-3, 3, n),
    })
    y_demo = (X_demo["age"] ** 2 + X_demo["income"] ** 2) < 4.0

    demo_regions = [
        [("age", 1.5, ">"), ("income", 1.0, ">")],
        [("age", -1.0, "<")],
        [("income", -1.5, "<"), ("age", 0.5, ">")],
    ]

    fig, ax = plot_2d_grid(
        X=X_demo,
        y=y_demo,
        x1_lim=(-3.5, 3.5),
        x2_lim=(-3.5, 3.5),
        regions=demo_regions,
        n_gridlines=(4, 6),
        title="Example: Age vs Income (long title that might wrap around)",
    )
    plt.savefig("/mnt/user-data/outputs/demo.png", dpi=150)
    plt.show()