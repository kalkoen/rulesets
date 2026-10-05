import pandas as pd

from scripts.results import *
from scripts.tex import aligned_value_parentheses_table

if __name__ == "__main__":
    experiment_name = "PDL1_experiment_1_cross"

    # Path to your folder containing the json files
    data_folder = f"data/{experiment_name}/"
    binarized_folder = f"data/{experiment_name}/binarized"
    target_folder = f"data/{experiment_name}/results"

    columns = {
        "yq": int,
        "s": int,
        "xnq": int
    }

    results_dict = get_results_from_folder(target_folder, r"^y_q_(\d+)_s_(\d+)_X_nq_(\d+)\.json$")
    results_df = get_results_df(results_dict, columns)

    pd.set_option("display.max_rows", 1000)
    print(results_df.groupby(["xnq", "yq"]).count())


    results_X_y_train = gather_results_X_y(results_dict,
                                     ("name", "index"),
                                     os.path.join(binarized_folder, "y_q_{0}_s_{1}_X_nq_{2}_train_X.csv"),
                                     os.path.join(binarized_folder, "y_q_{0}_s_{1}_train_y.csv"))

    results_X_y_test = gather_results_X_y(results_dict,
                                     ("name", "index"),
                                     os.path.join(binarized_folder, "y_q_{0}_s_{1}_X_nq_{2}_test_X.csv"),
                                     os.path.join(binarized_folder, "y_q_{0}_s_{1}_test_y.csv"))

    accuracies_train = get_performance_metrics(results_X_y_train,
                                   columns,
                                   performance_metric_dict={
                                       "Accuracy (\\%) train": accuracy_score,
                                   },
                                   )
    accuracies_test = get_performance_metrics(results_X_y_test,
                                   columns,
                                   performance_metric_dict={
                                       "Accuracy (\\%) test_grid": accuracy_score,
                                   },
                                   )

    accuracy_table_train = get_mean_std_table(accuracies_train.drop("s", axis=1),
                                              ["xnq", "yq"],
                                              group_name=["xnq", "yq"])
    accuracy_table_test = get_mean_std_table(accuracies_test.drop("s", axis=1),
                                             ["xnq", "yq"],
                                             group_name=["xnq", "yq"])

    pd.set_option('display.max_columns', 1000)

    unstacked_accuracy_train = accuracy_table_train.unstack("xnq")
    unstacked_accuracy_train.columns = unstacked_accuracy_train.columns.droplevel(0)
    unstacked_accuracy_train.columns.name = "Xnq"

    unstacked_accuracy_test = accuracy_table_test.unstack("xnq")
    unstacked_accuracy_test.columns = unstacked_accuracy_test.columns.droplevel(0)
    unstacked_accuracy_test.columns.name = "Xnq"

    aligned_value_parentheses_table(unstacked_accuracy_train,
                                    filename=f"{experiment_name}/tab_accuracy_train.tex",
                                    caption=None,
                                    label=None)


    aligned_value_parentheses_table(unstacked_accuracy_test,
                                    filename=f"{experiment_name}/tab_accuracy_test.tex",
                                    caption=None,
                                    label=None)

    # Optional: Save to CSV
    # results_df.to_csv("objectives_summary.csv", index=False)
