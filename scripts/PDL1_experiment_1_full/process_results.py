import pandas as pd

from scripts.results import *

from scripts.tex import tex_gen_file

if __name__ == "__main__":
    experiments = ["PDL1_experiment_1_full",
                   "PDL1_experiment_1",
                   "PDL1_experiment_1_full",
                   "PDL1_experiment_1_neg"]

    data_experiment = "PDL1_experiment_1"

    file_patterns = [
        r"^Xnq_(\d+)_yq_(\d+)_full\.json$",
        r"^Xnq_(\d+)_yq_(\d+)\.json$",
        r"^Xnq_(\d+)_yq_(\d+)_neg_full\.json$",
        r"^Xnq_(\d+)_yq_(\d+)_neg\.json$",
    ]

    X_filename_formats = [
        "X_nq_{0}.csv",
        "X_nq_{0}.csv",
        "X_nq_{0}_neg.csv",
        "X_nq_{0}_neg.csv",
    ]

    columns = {
        "Xnq": int,
        "yq": int
    }


    binarized_folder = f"data/{data_experiment}/binarized"

    folders = [
        f"data/{name}/results" for name in experiments
    ]

    results_dict = [
        get_results_from_folder(folder, pattern) for folder, pattern in zip(folders, file_patterns)
    ]

    results_df = [get_results_df(results_dict, columns) for results_dict in results_dict]

    results_X_y = [gather_results_X_y(results_dict,
                                      columns,
                                      os.path.join(binarized_folder, X_format),
                                      os.path.join(binarized_folder, "y_q_{1}.csv")
                                      )
                   for results_dict, X_format in zip(results_dict, X_filename_formats)]

    metrics = [get_default_metrics_binary(rd, rdf, rXy, columns) for
               rd, rdf, rXy in zip(results_dict, results_df, results_X_y)]

    metric_names = metrics[0].columns

    for metric_name in metric_names:
        columns = [
            metrics_df[metric_name]
            for experiment_name, metrics_df in zip(experiments, metrics)
        ]
        for col, experiment_name in zip(columns, experiments):
            col.name = experiment_name
        metric_df = pd.concat(columns, axis=1).sort_index()
        metric_df = metric_df.dropna(subset=["PDL1_experiment_1_full"], how='all')

        metric_df.columns = [
            "Full without negated",
            "CG without negated",
            "Full with negated",
            "CG with negated"
        ]

        metric_df.columns = [
            f"\\makecell{{{col.replace(' ', ' \\\\ ')}}}"
            for col in metric_df.columns
        ]

        clean_metric = metric_name.replace(' ', '_')
        filename = f"PDL1_experiment_full/tab_{clean_metric}.tex"

        metric_df.to_latex(
            tex_gen_file(filename),
            float_format="{:,.3g}".format,
            caption=None,
            label=None,
            column_format='rrrrrr',
            escape=False
        )
        pd.set_option("display.max_columns", 1000)



