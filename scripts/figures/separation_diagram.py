import matplotlib.pyplot as plt

from tex import tex_gen_file

plt.rcParams['font.size'] = 14
fig, ax = plt.subplots(figsize=(10, 3))
margin = 0.1
mx = 2
mn = 0
gamma_1 = 1.8
gamma_2 = 1.5
gamma_3 = 0.9
gamma_4 = 0.5
gamma_5 = 0.2
y_positions = [1, 0.5, 0]
# irregular positions -> names
tick_data = [
    (2, r"$\gamma_0 = 2$"),
    (gamma_5, r"$\gamma_5$"),
    (gamma_4, r"$\gamma_4$"),
    (gamma_3, r"$\gamma_3$"),
    (gamma_2, r"$\gamma_2$"),
    (gamma_1, r"$\gamma_1$"),
]
ticks = [pos for pos, name in tick_data]
tick_labels = [name for pos, name in tick_data]

# placeholder ticks for the new middle row -- fill in real values later
tick_data_mid = [
    (gamma_5, r"$\tau^*(5)=5$"),
    (gamma_4, r"$\tau^*(4)=5$"),
    (gamma_3, r"$\tau^*(3)=5$"),
    (gamma_2, r"$\tau^*(2)=2$"),
    (gamma_1, r"$\tau^*(1)=2$"),
]
ticks_mid = [pos for pos, name in tick_data_mid]
labels_mid = [name for pos, name in tick_data_mid]

for y in y_positions:
    if y == 0.5:
        # ax.plot(ticks_mid, [y]*len(ticks_mid), 'k|', markersize=10, zorder=2)
        for pos, lab in tick_data_mid:
            ax.text(pos, y+0.15, lab, ha='center', va='top')
    else:
        ax.axhline(y=y, xmin=0.05, xmax=0.95, color='black', lw=1.5, zorder=1)
        ax.plot(ticks, [y]*len(ticks), 'k|', markersize=10, zorder=2)

gamma_ranges = [
    (0, gamma_1, 2, r"$\gamma_0 - \gamma_1$"),
    (0, gamma_2, gamma_1, r"$\gamma_1 - \gamma_2$"),
    (0, gamma_3, gamma_2, r"$\gamma_2 - \gamma_3$"),
    (0, gamma_4, gamma_3, r"$\gamma_3 - \gamma_4$"),
    (0, gamma_5, gamma_4, r"$\gamma_4 - \gamma_5$"),
]
for axis_idx, start, end, label in gamma_ranges:
    y = y_positions[axis_idx]
    ax.text((start+end)/2, y + 0.1, label, ha='center')

# Example ranges to mark: (axis_index, start, end, label, color)
ranges = [
    (2, gamma_2, 2, r"$\overline{\gamma}_{\tau_1}$", "C1"),
    (2, gamma_5, gamma_2, r"$\overline{\gamma}_{\tau_2}$", "C2"),
]
for axis_idx, start, end, label, color in ranges:
    y = y_positions[axis_idx]
    ax.plot([start, end], [y, y], color=color, lw=4, solid_capstyle='butt', zorder=3)
    ax.text((start+end)/2, y + 0.1, label, ha='center')

ax.set_xlim(mn-margin, mx+margin)
ax.set_ylim(-0.5, 1.5)
ax.set_yticks([])
ax.set_xticks(ticks)
ax.set_xticklabels(tick_labels, rotation=0)
ax.spines[['top', 'right', 'left', 'bottom']].set_visible(False)
ax.set_title(r"Example case with $\tau_1 = 2, \tau_2 = 5$")
plt.tight_layout()
plt.savefig(tex_gen_file("separation.svg"))
plt.show()