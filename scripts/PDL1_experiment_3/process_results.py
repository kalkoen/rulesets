from PDL1_experiment_2.process_results import process_pdl1_experiment_2

process_pdl1_experiment_2("PDL1_experiment_3",
                          result_file_pattern=r"yq_{yq}_mw_{mw}_C_{C}_settings_D_{D}.set.json",
                           X_filename_format="X_broad.csv",
                           y_filename_format="y/y_broad_q_{yq}.csv")