import matplotlib.pyplot as plt
from matplotlib import ticker


def plot_steps(df, x='time', y='red_cost', hue='Xnq', units='yq', alpha=1.0):
    """
    Plots the dataframe using manual Matplotlib loops.
    Groups by 'hue' and 'units' to draw 100+ individual lines.
    """
    plt.figure(figsize=(8, 5))
    ax = plt.gca()

    # 1. Setup Color Mapping (The 'hue' equivalent)
    unique_hues = df[hue].unique()
    # colors = plt.cm.get_cmap('tab10', len(unique_hues))
    # color_map = {val: colors(i) for i, val in enumerate(unique_hues)}

    # 2. Manual Grouping and Plotting (The 'units' equivalent)
    # We group by both to get every individual line path
    for (hue_val, unit_val), group in df.groupby([hue, units]):
        ax.plot(
            group[x],
            group[y],
            color="blue",
            alpha=alpha,
            linewidth=1
        )

    # 3. Apply 'Nice' Symlog Logic
    ax.set_yscale('symlog', linthresh=1.0)

    # 4. Professional Grid Lines
    ax.yaxis.set_major_locator(ticker.SymmetricalLogLocator(base=10, linthresh=1.0))
    ax.yaxis.set_minor_locator(ticker.SymmetricalLogLocator(base=10, linthresh=1.0, subs=range(1, 10)))

    ax.grid(True, which="major", axis="y", color='gray', linestyle='-', linewidth=0.7, alpha=0.5)
    ax.grid(True, which="minor", axis="y", color='gray', linestyle=':', linewidth=0.4, alpha=0.3)
    ax.grid(True, axis="x", alpha=0.2)

    plt.xlabel(x.capitalize())
    plt.ylabel(y.replace('_', ' ').capitalize())
    plt.title(f"{y.replace('_', ' ').capitalize()} over {x}")

    # plt.show(block=False, dpi=600)