import os
import json
import re
import pandas as pd
from sklearn.metrics import accuracy_score

import tex
from results import *
from tex import aligned_value_parentheses_table

if __name__ == "__main__":
    # Path to your folder containing the json files
    data_folder = "data/benchmark/"
    binarized_folder = "data/benchmark/binarized"
    results_folder = "data/benchmark/results"

    columns = {
        "name": str,
        "index": int
    }

    results_dict = get_results_from_folder(results_folder, "{name}_{index}.json", columns)
    assert len(results_dict) > 0

    results_df = get_results_df(results_dict, columns)

    objective_table = get_mean_std_table(results_df,
                                         "name",
                                         "Dataset",
                                         "obj",
                                         "Objective")

    results_X_y_train = iterator_X_y(results_dict,
                                     binarized_folder,
                                     "{name}_{index}_train_X.csv",
                                     "{name}_{index}_train_y.csv")

    results_X_y_test = iterator_X_y(results_dict,
                                    binarized_folder,
                                    "{name}_{index}_test_X.csv",
                                    "{name}_{index}_test_y.csv")

    accuracies_train = get_performance_metrics(results_X_y_train,
                                               columns,
                                               performance_metric_dict={
                                                   r"Train acc. (\%)": accuracy_score,
                                               },
                                               predict_func=predict_binary
                                               )
    accuracies_test = get_performance_metrics(results_X_y_test,
                                              columns,
                                              performance_metric_dict={
                                                  r"Test acc. (\%)": accuracy_score,
                                              },
                                              predict_func=predict_binary
                                              )

    accuracy_table_train = get_mean_std_table(accuracies_train.drop("index", axis=1),
                                              "name",
                                              group_name="Dataset")
    accuracy_table_test = get_mean_std_table(accuracies_test.drop("index", axis=1),
                                             "name",
                                             group_name="Dataset")

    results_df["soltime"] /= 100
    results_df["soltime_cg"] /= 100
    solve_time_table = get_mean_std_table(results_df,
                                          "name",
                                          "Dataset",
                                          "soltime_cg",
                                          "Solve time (s)",
                                          decimals=0)

    my_table = pd.concat([accuracy_table_train, accuracy_table_test, solve_time_table], axis=1)

    lawless_table = pd.read_csv(os.path.join(data_folder, "lawless.csv")).set_index("Dataset")

    final_table = my_table.join(lawless_table, how="left")

    final_table = final_table.reindex(sorted(final_table.columns), axis=1)

    final_table.index = final_table.index + "~" + final_table["Citation"]
    final_table = final_table.drop(columns=["Citation"])
    final_table.index.name="Dataset"

    pd.set_option('display.max_columns', 1000)

    aligned_value_parentheses_table(final_table,
                                    filename="benchmark/tab_accuracy.tex",
                                    caption=None,
                                    label=None,
                                    exclude_columns=['$C$'])

    # Optional: Save to CSV
    # results_df.to_csv("objectives_summary.csv", index=False)
