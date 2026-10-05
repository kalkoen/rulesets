from typing import Literal

import numpy as np
import matplotlib.pyplot as plt

from scripts.tex import tex_gen_file


def plot_loss_function(ax, title, z, loss, alpha, beta, y ):
    ax.plot(z, loss, linewidth=2, color="black" if y == 1 else "darkred")
    ax.set_title(title, loc='left', fontsize=12)
    ax.set_ylabel('Loss')

    # Add reference lines
    ax.axvline(x=alpha - beta, color='gray', linestyle='--')
    ax.axvline(x=alpha, color='gray', linestyle='--')
    ax.axvline(x=alpha + beta, color='gray', linestyle='--')

    # Cleanup
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    ax.set_yticks([0, 1, 2])

def complete_loss_function_plot(ax, alpha, beta):
    # Set custom x-axis labels
    ax.set_xticks([alpha - beta, alpha, alpha + beta])
    ax.set_xticklabels([r'$\alpha - \beta$', r'$\alpha$', r'$\alpha + \beta$'], fontsize=14)
    ax.set_xlabel("$X_{i j}$", fontsize=14)

    # Ensure y-axis range is exactly 0 to 2
    ax.set_ylim(0, 2.5)
    ax.set_xlim(-1 + alpha - beta, alpha + beta + 1)



def plot_loss_functions(y: Literal[-1,1], corrected: bool):
    """Generates loss plots with ramp loss, alpha labels, and fixed y-limits."""
    alpha = 0
    beta = 2
    z = np.linspace(-beta-1, beta+1, 5000)

    assert y == 1 or y == -1


    if not corrected:
        linear_slope = (-y * (z - alpha)) / beta
        hinge_loss = np.maximum(0, linear_slope)
        ramp_loss = np.minimum(1, np.maximum(0, linear_slope))
        zero_one_loss = np.where(linear_slope > 0, 1, 0)

    else:
        linear_slope = (-y * (z - (alpha + y*beta))) / beta
        hinge_loss = np.maximum(0, linear_slope)
        ramp_loss = np.minimum(2, np.maximum(0, linear_slope))
        zero_one_loss = np.where(linear_slope > 1, 1, 0)

    # Setup the figure
    fig, axes = plt.subplots(3, 1, figsize=(6, 10), sharex=True, sharey=True)

    data = [
        ("(a)", hinge_loss),
        ("(b)", ramp_loss),
        ("(c)", zero_one_loss)
    ]

    for ax, (title, loss) in zip(axes, data):
        plot_loss_function(ax, title, z, loss, alpha, beta, y)
    complete_loss_function_plot(axes[2], alpha, beta)

    plt.tight_layout()
    fig.savefig(tex_gen_file(f"loss_functions_y_{y}{"_corrected" if corrected else ''}.svg"))
    plt.show()

    # fig, ax = plt.subplots(1, 1, figsize=(6, 4), sharex=True, sharey=True)
    # plot_loss_function(ax, "", z, ramp_loss, alpha, beta)
    # ax.plot(z, zero_one_loss, 'k', linewidth=2, alpha=0.2)
    # ax.fill_between(z, ramp_loss, zero_one_loss,
    #                 where=(zero_one_loss > ramp_loss),
    #                 facecolor="none",
    #                 edgecolor="gray",
    #                 hatch='///',
    #                 linewidth=0)
    # complete_loss_function_plot(ax, alpha, beta)
    # ax.set_ylim(0, 1.5)
    # plt.tight_layout()
    # fig.savefig(tex_gen_file("ramp_loss_reduction.svg"))
    # plt.show()



# Execute the function
plot_loss_functions(-1, False)
plot_loss_functions(1, False)
plot_loss_functions(-1, True)
plot_loss_functions(1, True)