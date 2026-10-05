import pandas as pd
import seaborn as sns
from matplotlib import pyplot as plt

from scripts.results import *
from scripts.tex import aligned_value_parentheses_table

if __name__ == "__main__":
    experiment_name = "PDL1_experiment_1_C"
    data_experiment_data = "PDL1_experiment_1_cross"

    # Path to your folder containing the json files
    data_folder = f"data/{data_experiment_data}/"
    binarized_folder = os.path.join(data_folder, "binarized")

    target_folder = f"data/{experiment_name}/results"

    columns = {"yq": int,
               "s": int,
               "xnq": int,
               "C": int}

    results_dict = get_results_from_folder(target_folder, r"^y_q_(\d+)_s_(\d+)_X_nq_(\d+)_(\d+)\.json$")
    results_df = get_results_df(results_dict, columns)


    pd.set_option("display.max_rows", 1000)
    pd.set_option("display.max_columns", 1000)

    # print(results_df.groupby(["xnq", "yq", "s"]).count())

    results_X_y_train = gather_results_X_y(results_dict,
                                     columns,
                                     os.path.join(binarized_folder, "y_q_{0}_s_{1}_X_nq_{2}_train_X.csv"),
                                     os.path.join(binarized_folder, "y_q_{0}_s_{1}_train_y.csv"))

    results_X_y_test = gather_results_X_y(results_dict,
                                     columns,
                                     os.path.join(binarized_folder, "y_q_{0}_s_{1}_X_nq_{2}_test_X.csv"),
                                     os.path.join(binarized_folder, "y_q_{0}_s_{1}_test_y.csv"))

    computed_metrics = get_computed_metrics(results_dict, columns, default_computed_metrics())

    computed_metrics_entries = computed_metrics.melt(
        id_vars=columns,
        var_name='metric',
        value_name='value')

    accuracies_train = get_performance_metrics(results_X_y_train,
                                   columns,
                                   performance_metric_dict={
                                       "Accuracy": accuracy_score,
                                   },
                                   )
    accuracies_train["Type"] = "Train"
    accuracies_test = get_performance_metrics(results_X_y_test,
                                   columns,
                                   performance_metric_dict={
                                       "Accuracy": accuracy_score,
                                   },
                                   )
    accuracies_test["Type"] = "Test"


    accuracies = pd.concat([accuracies_train, accuracies_test], axis=0).reset_index()
    g = sns.relplot(
        data=accuracies,
        x='C',
        y='Accuracy',
        hue='Type',  # Different colors for each Type
        col='yq',  # Split into columns based on yq
        row='xnq',  # Split into rows based on xnq
        kind='line',  # Specify line plot
        marker='o',  # Add markers to clearly see individual data points
        aspect=2
    )
    plt.show()

    g = sns.relplot(
        data=computed_metrics_entries,
        x='C',
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
