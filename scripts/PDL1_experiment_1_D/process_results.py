import pandas as pd
import seaborn as sns
from matplotlib import pyplot as plt

from scripts.results import *
from scripts.tex import aligned_value_parentheses_table

if __name__ == "__main__":
    experiment_name = "PDL1_experiment_1_D"
    data_experiment_data = "PDL1_experiment_1"

    # Path to your folder containing the json files
    data_folder = f"data/{data_experiment_data}/"
    binarized_folder = os.path.join(data_folder, "binarized")

    target_folder = f"data/{experiment_name}/results"

    columns = {"xnq": int,
               "yq": int,
               "D": int}

    results_dict = get_results_from_folder(target_folder, r"^Xnq_(\d+)_yq_(\d+)_settings_D_(\d+).set\.json$")

    results_df = get_results_df(results_dict, columns)

    pd.set_option("display.max_rows", 1000)
    pd.set_option("display.max_columns", 1000)

    # print(results_df.groupby(["xnq", "yq", "s"]).count())

    results_X_y = gather_results_X_y(results_dict,
                                     columns,
                                     os.path.join(binarized_folder, "X_nq_{0}_neg.csv"),
                                     os.path.join(binarized_folder, "y_q_{1}.csv"))

    computed_metrics = get_computed_metrics(results_dict, columns, default_computed_metrics())

    computed_metrics_entries = computed_metrics.melt(
        id_vars=columns,
        var_name='metric',
        value_name='value')

    accuracies = get_performance_metrics(results_X_y,
                                   columns,
                                   performance_metric_dict={
                                       "Accuracy": accuracy_score,
                                   },
                                   )

    g = sns.relplot(
        data=accuracies,
        x='D',
        y='Accuracy',
        col='yq',  # Split into columns based on yq
        row='xnq',  # Split into rows based on xnq
        kind='line',  # Specify line plot
        marker='o',  # Add markers to clearly see individual data points
        aspect=2,

    )
    g.set(ylim=(0, 1))
    plt.show()

    g = sns.relplot(
        data=computed_metrics_entries,
        x='D',
        y='value',
        hue='metric',  # Different colors for each Type
        col='yq',  # Split into columns based on yq
        row='xnq',  # Split into rows based on xnq
        kind='line',  # Specify line plot
        marker='o',
        aspect=2# Add markers to clearly see individual data points
    )

    plt.show()
    # Optional: Save to CSV
    # results_df.to_csv("objectives_summary.csv", index=False)
