from PDL1_experiment_1.process_results import process_pdl1_experiment_1

if __name__ == "__main__":
     process_pdl1_experiment_1("PDL1_experiment_1_neg",
                                             "PDL1_experiment_1",
                                             result_file_pattern=r"Xnq_{Xnq}_yq_{yq}_neg.json",
                                             X_filename_format="X_nq_{Xnq}_neg.csv",
                                             y_filename_format="y_q_{yq}.csv"
                                             )