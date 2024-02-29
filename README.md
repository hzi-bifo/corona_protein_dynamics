# Pipeline for S-protein dynamics
This repository hosts code for computing the mutation dynamics of S-protein in SARS-Cov-2 based on phylogenetic and statistical analyses.

## Installation

```
git clone https://github.com/hzi-bifo/corona_protein_dynamics.git
cd corona_protein_dynamics
conda env create -f environment.yml
```

## Usage

```shell
Usage: bash corona_sd_plot.sh -r <root_path> \
  [-s <sample_size>] \
  [-t <time_period>] \
  [-g <location>] \
  -c <genomes_file> \
  -l <lineage_file>
Options:
  -r          Path to the root (reference sequence)
  -s          Number of sampled sequences (if not set, no sampling)
  -t          Analyze by given time period (year, month, day, season)
  -g          Sample per geographic location (continent, country). If fasta is from one location, no need to specify
  -c          Get coding sequences first (specify genomes file)
  -l          Map substitutions to pangolin lineages (specify lineage file)
  -h, --help  Show help
```

After uncompress the test data in test_data/S_Germany with:
```shell
pigz -d test_data/S_Germany/DE.metadata.tsv.gz test_data/S_Germany/DE.fasta.gz
```

You can run the pipeline on test data:
```shell
bash corona_sd_plot.sh \
  <output dir> \
  -r root_seq/Asia_root_cds.fa \
  -t month \
  -c test_data/S_Germany/DE.fasta \
  -l test_data/S_Germany/DE.metadata.tsv
```

This will start the pipeline in the `<output dir>` folder you have created, use the root sequence from file `root_seq/Asia_root_cds.fa` and test data for Germany, and make a plot with monthly time periods.

The `-l` option requires a metadata file as input (e.g., `-l DE.metadata.tsv`) to map amino acid substitutions to pangolin lineages. The `metadata.tsv` file should adhere to the format exemplified in `test_data/S_Germany/DE.metadata.tsv`

The script requires sequences to have header in the following format:
```shell
>Germany/BY-ChVir-1017/2020|EPI_ISL_450209|2020-01-30
```
