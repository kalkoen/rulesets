import os

from ucimlrepo import fetch_ucirepo

from data import dump_splits

from binarizer import binarize_data

random_state = 42

n_quantiles = 9
n_splits = 10
folder_out_split = "data/benchmark/split"
folder_out_bin = "data/benchmark/binarized"
out_settings = "data/benchmark/settings.set"
folder_out_results = "data/benchmark/results_uncorrected"

datasets = [
    (267, "banknote", None),
    (45, "heart", {
        0: False,
        1: True,
        2: True,
        3: True,
        4: True
    }),
    (225, "ILPD", {
        1: False,
        2: True
    }),
    (52, "ionosphere", {
        "g": False,
        "b": True
    }),
    # (34, "pima"),
    (101, "tic-tac-toe", {
        "positive": True,
        "negative": False
    }),
    (176, "transfusion", None),
    (17, "WDBC", {
        "B": False,
        "M": True
    }),
]

settings = """
rs/method = 1
rs/cg_timelimit = 300"""

for folder in [folder_out_split, folder_out_bin]:
    assert not os.path.exists(folder), "You must first remove all relevant folders"
    os.makedirs(folder)

with open(out_settings, "w") as file:
    file.write(settings)

for id, name, ymap in datasets:
    # fetch dataset
    dataset = fetch_ucirepo(id=id)

    # data (as pandas dataframes)
    X = dataset.data.features
    y = dataset.data.targets


    print("Preparing dataset. id:", id, "name:", name, "ymap:", ymap)
    # print("> X")
    # print(X)
    # print("> y")
    # print(y)
    # print("> value counts")
    # print(y[y.columns[0]].value_counts())

    assert(len(y.columns) == 1)
    if ymap is not None:
        colname = y.columns[0]
        y[colname] = y[colname].map(ymap)

    X_cleaned_interim = X.dropna(how='any')
    y_cleaned_interim = y.dropna()
    common_index = X_cleaned_interim.index.intersection(y_cleaned_interim.index)
    X = X.loc[common_index]
    y = y.loc[common_index]
    X = X.reset_index(drop=True)
    y = y.reset_index(drop=True)

    train_splits, test_splits = dump_splits(X, y, n_splits, folder_out_split, name, random_state=random_state)

    if not os.path.exists(folder_out_split):
        os.mkdir(folder_out_split)
    if not os.path.exists(folder_out_bin):
        os.mkdir(folder_out_bin)

    for (dname_train, (X_train, y_train)), (dname_test, (X_test, y_test)) \
            in zip(train_splits.items(), test_splits.items()):

        X_bin_train, [X_bin_test] = binarize_data(X_train, [X_test], n_quantiles, True, 2)

        X_bin_train.astype(int).to_csv(f"{folder_out_bin}/{dname_train}_train_X.csv", index=False)
        X_bin_test.astype(int).to_csv(f"{folder_out_bin}/{dname_train}_test_X.csv", index=False)
        y_train.astype(int).to_csv(f"{folder_out_bin}/{dname_test}_train_y.csv", index=False)
        y_test.astype(int).to_csv(f"{folder_out_bin}/{dname_test}_test_y.csv", index=False)


with open(out_settings, "w", encoding="utf-8") as file:
    file.write(settings)