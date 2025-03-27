# Pipeline for S-protein dynamics anslysis

This repository hosts code for computing the mutation dynamics of S-protein in SARS-Cov-2 based on phylogenetic and statistical analyses.

## Argument and parameters

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

### There are two ways to install this pipeline
  This pipeline has been only tested on **Linux** system.
  
  1. [Install it from the source in this repository](#i-installation-with-micromamba).
  1. [Use singularity](#ii-use-singularity-container).



> [!IMPORTANT]  
> Singularity image guarantees reproducibility by encapsulating exact package builds. To avoid potential licensing issues with the Conda defaults channel—which now requires a license for commercial use—we have removed it from our environment YAML file. As a result, the package builds provided solely by the community channels (conda-forge and bioconda) may differ from those originally set up in our pipeline.
> If you enconter any issues with installation and running the pipeline, please refer to [Command issues](#common-errors-or-issues).


## I. Installation with micromamba

As **`micromamba`** is much faster than `conda` to resolve the dependencies, we recommend to use it to create the environment. Check [here](https://mamba.readthedocs.io/en/latest/installation/micromamba-installation.html), if you don't know how to install micromamba. 
If you still prefer conda, just replace` micromamba` with `conda` in the command lines.

> [!IMPORTANT]
> Before installation please remove the `defaults` channel from the `~/.mambarc`, `~/.condarc` files.

### 1. Clone the repo source code and create conda env

```shell
git clone https://github.com/hzi-bifo/corona_protein_dynamics.git
cd corona_protein_dynamics
micromamba env create -f environment.yml
```

### 2. Compile `phylogeo-tools` (optional, needed only when the pre-built does not work)
If the pre-built binaries (`phylogeo_sankoff_general_dna`, `mutation-samples` and `tree-build-as-pangolin`) in `libs/phylogeo-tools/build` do not work, they should be built with the following commands. Please don't build them from source code if the pre-built binaries work fine.

- 2.1 Clean and clone the phylogeo-tools repo into `libs/phylogeo-tools`

```shell
rm -rf libs/phylogeo-tools
git clone https://github.com/hzi-bifo/phylogeo-tools.git libs/phylogeo-tools
cd libs/phylogeo-tools
mkdir -p build/
```

- 2.2 Create conda env for compilation and compile the code

```shell
micromamba create -n cxx -c conda-forge cxx-compiler c-compiler libstdcxx-ng boost boost-cpp libarchive
micromamba activate cxx
export LD_LIBRARY_PATH=$CONDA_PREFIX/lib
make
cd ../../
micromamba deactivate
```

### 3. Uncompress the test data in `test_data/Germany`

```shell
gunzip test_data/Germany/DE.metadata.tsv.gz
gunzip test_data/Germany/DE.fasta.gz
```
> [!NOTE]
> The test data were created using sequences downloaded from NCBI, and the EPI_ISL accessions were artificially generated only for testing purposes.

### 4. Create output directory and activate the env 

```shell
mkdir -p <output dir>
micromamba activate corona_sd_plots
```

Make sure the <output dir> is empty and writable.

### 5. Run the pipeline on test data

```shell
bash corona_sd_plot.sh \
  <output dir> \
  -r root_seq/Asia_root_cds.fa \
  -t month \
  -c test_data/Germany/DE.fasta \
  -l test_data/Germany/DE.metadata.tsv
```

This will start the pipeline in the `<output dir>` folder you have created, use the root sequence from file `root_seq/Asia_root_cds.fa` and test data for Germany, and make a plot with monthly time periods. The runtime on this test data is around 30 minutes.

The `-l` option requires a metadata file as input (e.g., `-l DE.metadata.tsv`) to map amino acid substitutions to pangolin lineages. The `metadata.tsv` file should adhere to the format exemplified in `test_data/Germany/DE.metadata.tsv`

> [!NOTE]
> You might see the warning: `convert: command not found`. You can ignore this message, as the command is only used to convert plots in PDF to PNG format.


## II. Use singularity container

### 1. Pull the singularity image

```shell
micromamba install singularity=3.8.6
singularity remote add --no-login cloud https://cloud.sylabs.io
singularity pull --arch amd64 library://zldeng/collection/corona_protein_dynamics:latest
```
The image has been tested with singularity 3.8.6.

### 2. Download the test data

```shell
mkdir -p testdata
cd testdata
wget https://raw.githubusercontent.com/hzi-bifo/corona_protein_dynamics/refs/heads/new_sd_plots/test_data/Germany/DE.fasta.gz
wget https://raw.githubusercontent.com/hzi-bifo/corona_protein_dynamics/refs/heads/new_sd_plots/test_data/Germany/DE.metadata.tsv.gz
gunzip DE.fasta.gz
gunzip DE.metadata.tsv.gz
cd ..
```

### 3. Create the output directory

```shell
mkdir -p output
```

Make sure the `output` directory is empty and writable.

### 4. Run the container:

```shell
singularity exec corona_protein_dynamics_latest.sif \
  bash /opt/corona_protein_dynamics/corona_sd_plot.sh output \
  -r /opt/corona_protein_dynamics/root_seq/Asia_root_cds.fa \
  -t month \
  -c testdata/DE.fasta \
  -l testdata/DE.metadata.tsv
```

## File formats

> [!NOTE]
> If you run the pipeline on your own data, please prepare the genomics fasta file and the metadata file following the same format like the test data. 

### Header format of fasta file

  The script requires sequences to have header in the following format:

  ```
  >Germany/BY-ChVir-1017/2020|EPI_ISL_450209|2020-01-30
  ```
  
  | Field          | Description    | 
  | -------------- | -------------- | 
  | 1| sequence ID starts with country name|
  | 2| EPI_ISL accession|
  | 3| sequence collection date|

### Metadata file format

 > [!NOTE]  
 > This 22 columns TSV file is important for providing more information about each sequence. The sequence and metadata are linked by the EPI_ISL accession. Therefore, the EPI_ISL accession (second field of header) in the fasta header should be given also in the 3rd column in the metadata.
  
  This file is usually generated based on the metadata file from GISAID

  ```
  hCoV-19/Germany/HB-OY751659/2022	ncov	EPI_ISL_5751659	?	2022-12-13	Europe	Germany	Bremen	NA	Europe	Germany	Bremen	genome	29731	Human	unknown	unknown	?	BF.7	NA	NA	2022-12
  ```

  | Column          | Description   | Importance |
  | -------------- | -------------- | ---------  |
  | 1 | sequence ID starts with hCoV-19 then country name | important |
  | 2 | "ncov" | just put "ncov" |
  | 3 | EPI_ISL accession | important |
  | 4 | "?" | just put "?" |
  | 5 | collection date | important |
  | 6 | continent |  important for continent wise analysis |
  | 7 | country | important |
  | 8 | region | important for regional analysis |
  | 9 | "?" | just put "?" |
  | 10 | continent where the genome was sequenced | usually the same as col 6 |
  | 11 | country where the genome was sequenced | usually the same as col 7 |
  | 12 | region where the genome was sequenced | usually the same as col 8 |
  | 13 | "genome" | just put "genome" |
  | 14 | genome length | used to check genome quality |
  | 15 | virus host | just put "Human" |
  | 16 | "unknown" | just put "unknown" |
  | 17 | "unknown" | just put "unknown" |
  | 18 | "?" | just put "?" |
  | 19 |  Pangolin lineage assignment | required for mutation-lineage mapping |
  | 20 | "NA" | just put "NA" |
  | 21 | "NA" | just put "NA" |
  | 22 | collection month | required |
   

## Output files

There are two important output files

- `.significant_positions.pdf`

  The protein mutation dynamics plot with significant changes being marked

- `.mutation_lineage.summary.tsv` and `.mutation_lineage.summary.json`

  The fraction of each mutation contributed by different lineages

## Common errors or issues

1. `Pip subprocess error`, `CondaEnvException: Pip failed` when creating the env

   We recommend to use `micromamba`. And please make sure you don't have `defaults` in `~/.mambarc`, `~/.condarc` files.

2. `mktemp: failed to create directory via template` when running the singularity container

   Check if the directory defined in the `TMPDIR` environmental variable has writable permission for you. This error occurs usually when you are using a old version of singularity or apptainer version 1. Please install newer version of singularity with `micromamba install singularity=3.8.6`.
   
3. convert command not found

   You can ignore it as this command just convert plots from PDF to PNG format.

4. collect2: error: ld returned 1 exit status when compiling `phylogeo-tools`

   Your system's GLIBC is outdated. Please update it to a newer version. The update process will vary depending on your OS distribution.
   In most cases, you can use the pre-built binary in the libs folder without needing to build it manually. Only compile the code if the pre-built version does not work.
   If the pre-built binaries are not compatible with you OS and you could not compile them from source. You can use the Singularity image as describe [here](#ii-use-singularity-container).

5. expansion requires a literal when running with singularity

   This message can be ignored.

