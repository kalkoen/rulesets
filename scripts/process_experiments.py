import os
import subprocess
import argparse
import sys
from pathlib import Path

experiments = [
    # "benchmark",
    # "PDL1_experiment_1",
    # "PDL1_experiment_1_neg",
    # "PDL1_experiment_1_full",
    # "PDL1_experiment_1_cross",
    # "PDL1_experiment_1_D",
    # "PDL1_experiment_1_C",
    ("figures", "branch_examples"),
    ("figures", "loss_functions"),
    "test_clouds"
]

def main():
    parser = argparse.ArgumentParser(description="Run experiment scripts within a specific venv.")
    parser.add_argument(
        "--venv",
        default=sys.executable,
        help="Path to the venv python executable (default: current environment)"
    )
    args = parser.parse_args()

    for experiment in experiments:
        print(f"===== Processing {experiment} =====")

        if type(experiment) == str:
            script_path = f"scripts/{experiment}/process_results.py"
        else: script_path =f"scripts/{experiment[0]}/{experiment[1]}.py"

        if os.path.exists(script_path):
            try:
                # Use the provided venv or default to current sys.executable
                env = os.environ.copy()
                result = subprocess.run(
                    [args.venv, script_path],
                    env=env,
                    cwd=os.getcwd(),
                    check=True,
                    capture_output=True,
                    text=True
                )
                print(f"Successfully finished {experiment}")

                if result.stderr:
                    print(f"Warnings for {experiment}:\n{result.stderr}")

            except subprocess.CalledProcessError as e:
                print(f"Error processing {experiment}:")
                print(f"STDERR: {e.stderr}")
        else:
            print(f"Skipping {experiment}: Script not found.")


if __name__ == "__main__":

    sys.path.append(os.path.abspath(os.path.dirname(__file__)))

    main()