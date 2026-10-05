from matplotlib import pyplot as plt

from results import *


def process_pdl1_experiment_1(experiment_name,
                              data_experiment_name,
                              result_file_pattern=r"Xnq_{Xnq}_yq_{yq}.json",
                              X_filename_format="X_nq_{Xnq}.csv",
                              y_filename_format="y_q_{yq}.csv", ):
    binarized_folder = f"data/{data_experiment_name}/binarized"
    results_folder = f"data/{experiment_name}/results"

    columns = {
        "Xnq": int,
        "yq": int
    }

    results = get_results_from_folder(results_folder, result_file_pattern, columns)
    df_results = get_results_df(results, columns)

    results_X_y_pred = iterator_X_y_pred(
        results,
        binarized_folder,
        X_filename_format,
        y_filename_format,
        predict_binary)

    rows = []
    for idx, result, X, y, y_hat in results_X_y_pred:
        row = idx.copy()
        row["accuracy"] = accuracy_score(y, y_hat)
        row["precision"] = precision_score(y, y_hat)
        row["recall"] = recall_score(y, y_hat)

        rows.append(row)

    performance_metrics = pd.DataFrame(rows)
    performance_metrics["yq"] = (performance_metrics["yq"] + 1)/10

    performance_metrics.to_csv(os.path.join(results_folder, "performance_metrics.csv"))

    df_results.to_csv(os.path.join(results_folder, "records.csv"))




if __name__ == "__main__":
    process_pdl1_experiment_1("PDL1_experiment_1", "PDL1_experiment_1")
