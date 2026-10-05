import matplotlib.pyplot as plt
import numpy as np

from scripts.tex import tex_gen_file


def plot_grid(v2: bool):
    fig, ax = plt.subplots(figsize=(6, 6))

    # --- Axes setup ---
    xl, xu, yl, yu = 0, 4.2, 0, 4.2
    ax.set_xlim(xl, xu)
    ax.set_ylim(xl, xu)
    ax.set_xticks(range(0,5))
    ax.set_yticks(range(1,5))
    ax.set_aspect("equal")

    for spine in ax.spines.values():
        spine.set_visible(False)
    ax.tick_params(length=0)
    ax.grid(False)

    margin_kw = dict(color="gray", linewidth=0.8, linestyle=(0, (1, 3)), clip_on=False)

    # vertical lines run from 0 to 4+pad in y
    # horizontal lines run from 0 to 4+pad in x
    # To avoid a grid look, clip margin lines to NOT cross the perpendicular main lines.
    # Vertical margins for a (x=3): run full height (no perpendicular vertical line crosses them)
    # Horizontal margins for b (y=3): run full width
    # They will intersect — to avoid the grid look, simply don't draw margins
    # in the region beyond the plot where they'd visually cross.
    # Easiest: just draw margins only within [0,4] (no bleed), main lines bleed.

    # --- Vertical line x=0.25  (label: c) — right margin only ---
    ax.plot([0.25, 0.25], [yl, yu], color="black", linewidth=1.2, clip_on=False)
    if not v2:
        ax.plot([1.25, 1.25], [yl, yu], **margin_kw)   # margin, clipped to plot area
    ax.text(0.30, 4.05, "f", fontsize=11, clip_on=False)

    # --- Vertical line x=3  (label: a) ---
    ax.plot([3, 3], [yl, yu], color="black", linewidth=1.2, clip_on=False)
    if not v2:
        ax.plot([2, 2], [yl, yu], **margin_kw)
        ax.plot([4, 4], [yl, yu], **margin_kw)
    ax.text(3.05, 4.05, "a", fontsize=11, clip_on=False)

    # --- Horizontal line y=3  (label: b) ---
    ax.plot([xl, xu], [3, 3], color="black", linewidth=1.2, clip_on=False)
    if not v2:
        ax.plot([xl, xu], [2, 2], **margin_kw)
        ax.plot([xl, xu], [4, 4], **margin_kw)
    ax.text(4.05, 3.12, "b", fontsize=11, clip_on=False)

    if v2 :
        ax.plot([2.75, 2.75], [yl, yu], color="black", linewidth=1.2, clip_on=False)
        ax.text(2.60, 4.05, "c", fontsize=11, clip_on=False)

        # --- Horizontal line y=3  (label: b) ---
        ax.plot([xl, xu], [2.75, 2.75], color="black", linewidth=1.2, clip_on=False)
        ax.text(4.05, 2.55, "d", fontsize=11, clip_on=False)

    # --- Points ---
    # Point 1: open circle at (1.5, 1.5)
    ax.plot(1.5, 1.5, marker="o", markersize=8,
            markerfacecolor="white", markeredgecolor="black", markeredgewidth=1.5,
            zorder=5)
    ax.text(1.58, 1.58, "1", fontsize=10)

    # Point 2: closed circle at (2.5, 2.5)
    ax.plot(2.5, 2.5, marker="o", markersize=8,
            markerfacecolor="black", markeredgecolor="black", markeredgewidth=1.5,
            zorder=5)
    ax.text(2.58, 2.58, "2", fontsize=10)

    # Point 3: open circle at (3.25, 3.25)
    ax.plot(3.25, 3.25, marker="o", markersize=8,
            markerfacecolor="white", markeredgecolor="black", markeredgewidth=1.5,
            zorder=5)
    ax.text(3.33, 3.33, "3", fontsize=10)

    plt.tight_layout()


if __name__ == "__main__":
    plot_grid(False)
    plt.savefig(tex_gen_file("branch_ex1.svg"), bbox_inches="tight")
    plt.show()

    plot_grid(True)
    plt.savefig(tex_gen_file("branch_ex2.svg"), bbox_inches="tight")
    plt.show()