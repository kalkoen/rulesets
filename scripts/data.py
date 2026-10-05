from sklearn.model_selection import StratifiedKFold

def dump_splits(X, y, n_splits, folder_out, identifier, random_state=None):
    assert (X.index == y.index).all()
    skf = StratifiedKFold(n_splits=n_splits,random_state=random_state, shuffle=True)
    train_splits, test_splits = {}, {}
    for i, (train_index, test_index) in enumerate(skf.split(X, y)):
        dname = f"{identifier}_{i}"

        X_i = X.iloc[train_index].copy()
        X_i.to_csv(f"{folder_out}/{dname}_train_X.csv", index=False)
        y_i = y.iloc[train_index].copy()
        y_i.to_csv(f"{folder_out}/{dname}_train_y.csv", index=False)
        train_splits[dname] = (X_i, y_i)

        X_i = X.iloc[test_index].copy().reset_index(drop=True)
        X_i.to_csv(f"{folder_out}/{dname}_test_X.csv", index=False)
        y_i = y.iloc[test_index].copy().reset_index(drop=True)
        y_i.to_csv(f"{folder_out}/{dname}_test_y.csv", index=False)
        test_splits[dname] = (X_i, y_i)

    return train_splits, test_splits