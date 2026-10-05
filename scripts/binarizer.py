import warnings

import numpy as np
import pandas as pd

'''
Helper functions to binerize data columns
'''

def binarize_numeric_series_lb(series, n_quantiles, decimals):
    quantile_inputs = np.linspace(1/(n_quantiles+1), 1-1/(n_quantiles+1), n_quantiles)
    quantiles, indices = np.unique(series.quantile(quantile_inputs), return_index=True)
    quantile_inputs = quantile_inputs[indices]
    output_series = []
    mask_functions = {}
    thresholds = {}
    for q, val in zip(quantile_inputs, quantiles):
        new_series = (series >= val).copy()
        new_series.name = f"{series.name}>={val:.{decimals}f} Q({q:.{decimals}f})"
        output_series.append(new_series)
        mask_functions[(series.name, new_series.name)] =  lambda s, v=val: s >= v
        thresholds[new_series.name] = val
    return output_series, mask_functions, thresholds
    
def binarize_categorical_series(series):
    unique_values = series.unique()
    output_series = []
    mask_functions = {}
    for val in unique_values:
        new_series = (series == val).copy()
        new_series.name = f"{series.name}=={val}"
        output_series.append(new_series)
        mask_functions[(series.name, new_series.name)] =  lambda s, v=val : s == v
    return output_series, mask_functions

def binarize_data(X, dfs_mimic, n_quantiles, include_negated=False, decimals=2, verbose=False, return_thresholds=False):
    columns = []
    mask_functions = {}
    thresholds = {}

    #Loop through and binerize columns
    for col in X:
        series = X[col]
        if series.dtype in ['object', 'str']:
            if verbose: print('Column '+str(col)+': Applying 1 Hot Encoding.')
            new_cols, new_masks = binarize_categorical_series(series)
        elif series.dtype in ['float64','int64']:
            if verbose: print('Column '+str(col)+': Binarzing with specified numeric strategy.')
            new_cols, new_masks, thresholds = binarize_numeric_series_lb(series, n_quantiles, decimals)
        elif series.dtype == 'bool':
            if verbose: print('Column '+str(col)+': Copying to final dataframe.')
            new_cols, new_masks = series[col]
        else:
            warnings.warn('Column '+str(col)+' has unexpected data type '+str(series.dtype)+' not including in final dataframe.')
            return None

        columns.extend(new_cols)
        mask_functions.update(new_masks)

    binarized = pd.concat(columns, axis=1)
    binarized_mimic = []

    if dfs_mimic is not None:
        for df in dfs_mimic:
            mimic_columns = []
            for (col_name, bin_col_name), mask_function in mask_functions.items():
                series = mask_function(df[col_name]).copy()
                series.name = bin_col_name
                mimic_columns.append(series)
            binarized_mimic.append(pd.concat(mimic_columns, axis=1))

    if include_negated:
        def add_negated(df):
            negated_df = ~df
            negated_df.columns = [col_name.replace("==", "!=").replace(">=", "<")
                                  if "==" in col_name or ">=" in col_name
                                  else col_name for col_name in df]
            return pd.concat([df, negated_df], axis=1)

        binarized = add_negated(binarized)
        binarized_mimic = [add_negated(df) for df in binarized_mimic]

    if return_thresholds:
        return binarized, binarized_mimic, thresholds
    else:
        return binarized, binarized_mimic


def mimic_binarization(X, dfs_mimic):
    pass


                            