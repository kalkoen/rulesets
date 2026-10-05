# Partners In Crime: Attempting to Infer Combinatorial Regulation from Bulk Transcriptomics

This is the codebase behind my MSc thesis. I presented and defended this
The lastest version of the report can be found under `tex/out/main.pdf`.
The report will soon be available in the Eindhoven University of Technology library.

In the basis, this project implements the optimization framework described in
**["Interpretable and Fair Boolean Rule Sets via Column Generation"](https://arxiv.org/abs/2111.08466)**.
In particular, the base model without fairness constraints is considered.

The idea of this project was to predict high gene expression using Transcription Factor (TF) activity scores,
obtained from bulk transcriptomics.
The philosophy behind this is: if a collection of conjunctive rules (OR-OF-ANDS, ruleset) is capable
of predicting high gene expression, then the combinations of TF in these rules found 
may be of interest for future research into the combinatorial regulation of the target gene.

## Usage
To get started quickly, please refer to `example.ipynb`.
This will guide you through using the simple Python solver,
as well as the advanced solver.
In the case of Windows, a precompiled executable is available, making usage easy.

To use your own notebook, we highly advise placing it in this repository's root folder 
(where both `README.md` and `example.ipynb` are located)
to avoid issues.

Make sure to use a Python environment with the modules from `requirements.txt` installed.
The easiest way to do this is via the following command.

```bash
pip install -r requirements.txt
```

This installs the exact versions described in the file.
If you are attempting this in the (far) future, it may be preferred to install the 
latest versions of the packages rather than these versions.


## Planned features
* **Weight parameter for false positives / false negatives**: 
with this parameter, false positives can be chosen to weight heavier than false negatives or the other way around.

## Mathematical techniques

The approach produces human-readable classification models by approximating the solution to a large-scale integer program using Column Generation. 
It identifies the most predictive "rules" (conjunctions of features) by alternating between a Master Problem and specialized Pricing subproblems.

## Structure of the project
This repository holds several tools.

* **Advanced optimization method**: Implemented in C++ using the SCIP optimization suite
* **Simpler greedy method**: Implemented in Python.
* **High Performance Cluster setup**: necessary files and scripts to run experiments in bulk on the HPC.
* **Report**: LaTeX files of the thesis report.

The file/folder structure is as follows:
* `data/`: Base datasets, as well as experiment-specific datasets in subfolders.
* `hpc/`: Scripts to help run experiments on High Performance Cluster.
* `lib/`: C++ libraries for the advancedd optimization method
* `scripts/`: a collection of scripts (Python, bash) to prepare, run and analyze experiments.
Also holds the Python model and greedy solver under `py/`
* `src/`: C++ source code for the advanced solver
* `tex/`: LaTeX source code for the report.

# Building the advanced solver (C++)

### C++ prerequisities (advanced solver)

* **C++ Standard:** C++20
* **SCIP Optimization Suite 10:** [SCIP](https://scipopt.org/) (Must be installed and discoverable via CMake)
* **LP Solver:** SoPlex (usually bundled with SCIP). Other LP solvers may be used if configured within SCIP.
* **Bit Manipulation:** [Roaring Bitmaps](https://roaringbitmap.org/) (Included in `lib/`)
* **JSON**: [JSON](https://github.com/nlohmann/json) library (Included in `lib/`)

### Build Instructions

The project uses CMake for build management. Ensure SCIP is in your system path.

```bash
mkdir build && cd build
cmake ..
make -j
```

# To do
* Describe each setting of the advanced C++ solver.