import os
import pandas as pd

from binarizer import binarize_data

experiment_name = "PDL1_experiment_3"
folder_out = os.path.join("data/", experiment_name)

n_quantiles_y = 9
tf_list = [
    "AR",
    "FOS",
    "FOSL1",
    "FOXO1",
    "GATA2",
    "IRF1",
    "MYC",
    "NFATC1",
    "NFKB1",
    "RELA",
    "SOX2",
    "SPI1",
    "STAT1",
    "STAT2",
    "STAT3",
    "STAT4",
    "STAT6",
    "TBX21"
]

target_gene = "CD274"


tpm = pd.read_csv("data/large/rnaseq_tpm_20220624.csv", low_memory=False)
tpm.rename(columns={"Unnamed: 1": "gene"}, inplace=True)
tpm.set_index("model_id", inplace=True)
data_source = tpm.loc["data_source"].copy()

tpm.drop(["model_name", "dataset_name", "data_source", "gene_id"], inplace=True)
tpm.set_index("gene", inplace=True, drop=True)

tpm_transpose = tpm.transpose().astype(float).sort_index()

X_full = tpm_transpose[tf_list].copy()
X_full.index.name = "sample"

# print(X_full.head())
# print(X_full.info())
# pd.set_option("display.max_columns", None)
# print(X_full.describe())

y_full = tpm_transpose[[target_gene]].copy()
y_full.index.name = "sample"
# print(y_full)
# print(y_full.info())
# print(y_full.describe())

os.makedirs(os.path.join(folder_out, "y"), exist_ok=True)

for dataset in ["Broad", "Sanger"]:
    samples = data_source.index[data_source == dataset]

    X = X_full.loc[samples].copy()
    y = y_full.loc[samples].copy()
    X.index.name = "sample"
    y.index.name = "sample"

    X.to_csv(os.path.join(folder_out,f"X_{dataset.lower()}.csv"), index=False)
    X.to_csv(os.path.join(folder_out,f"X_{dataset.lower()}_with_sample.csv"), index=True)

    y.to_csv(os.path.join(folder_out,f"y_{dataset.lower()}.csv"), index=False)
    y.to_csv(os.path.join(folder_out,f"y_{dataset.lower()}_with_sample.csv"), index=True)

    y_sanger_bin, [] = binarize_data(y, [], n_quantiles_y, False, decimals=4)
    for i, column in enumerate(y_sanger_bin):
        y_sanger_bin[column].astype(int).to_csv(os.path.join(folder_out, f"y/y_{dataset.lower()}_q_{i}.csv"), index=False)
        y_sanger_bin[column].astype(int).to_csv(os.path.join(folder_out, f"y/y_{dataset.lower()}_q_{i}_with_sample.csv"), index=True)