
import process_results

import os
import pandas as pd
from pandas.errors import EmptyDataError


def validate_csv_data(folder_path):
    if not os.path.exists(folder_path):
        print(f"Path '{folder_path}' not found.")
        return

    for filename in os.listdir(folder_path):
        if filename.endswith('.csv'):
            file_path = os.path.join(folder_path, filename)

            try:
                # We use nrows=1 because we only need to see if at least one row exists
                df = pd.read_csv(file_path, nrows=1)

                if df.empty:
                    print(f"[NO DATA]  {filename}: Header found, but 0 rows of data.")
                else:
                    pass
                    # print(f"[OK]       {filename}: Data present.")

            except EmptyDataError:
                print(f"[EMPTY]    {filename}: File is totally empty (no header).")
            except Exception as e:
                print(f"[ERROR]    {filename}: {e}")


if __name__ == "__main__":
    # Update this path to your folder
    validate_csv_data('../../data/PDL1_experiment_1_cross/binarized')