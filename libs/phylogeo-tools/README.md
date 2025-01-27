# Tools for Phylogeography analysis of the trees and related data
This package contains several tools working on genomic and proteomics sequences as well as phylogenetic trees.

## Build
To build, just run 
```
make
```
at the root. 
Note that there are some parameters that should be setted correctly in the makefile. One is `CONDA_PREFIX` which
is used in compilation of some applications.

## Filtering a sequence file
Build it with
```
make bin/alignment-filter
```

Given a fasta sequence file (`[INPUT-SEQ-FILE]`) and a file containing the ids (`[INPUT-IDS]`), the following command filters samples with id included in the ids file.
```
bin/alignment-filter -a [INPUT-SEQ-FILE] -i [INPUT-IDS]  > [OUTPUT-SEQ-FILE]
```

Instead of ids file, the ids could be extracted from a metadata file (with the GISAID format) with replacing `-i` argument with `-m` argument as follows:
```
bin/alignment-filter -a [INPUT-SEQ-FILE] -m [INPUT-METADATA-FILE]  > [OUTPUT-SEQ-FILE]
```

The header for each sequence in the output fasta file will contain only the id of the input sequence. It can be changed by the `-f` argument. The argument could contain "{0}", "{1}", "{2}", representing id, name, and date of the sequence, extracted from the input fasta file (not the metadata file). 
```
bin/alignment-filter -a [INPUT-SEQ-FILE] -i [INPUT-IDS] -f "[HEADER-FORMAT]" > [OUTPUT-SEQ-FILE]
```
For example, format "{0}|{1}|{2}" can produce the following header 
```
>EPI_ISL_999999|Wuhan/WIV01/2019|2019-01-01
```

A useful option for this tool is that the tool can directly read a compressed input fasta file, without uncompressing it. For this, the `-z` argument should be provided and the path of the fasta file inside the compressed file should be provided with the input compressed file separated by a colon.
```
bin/alignment-filter -a [INPUT-COMPRESSED-SEQ-FILE]:[INTERNAL-PATH-TO-FASTA-FILE] -z -i [INPUT-IDS] -f "[HEADER-FORMAT]" > [OUTPUT-SEQ-FILE]
```
For example, the `tar.xz` genomic sequences file of the SARS-CoV-2 obtained from GISAID contains three files, one of them is the actual fasta file with path `spikenuc1107.fasta`. For this, we can run the tool with the following `-a` part :
```
bin/alignment-filter -a [INPUT-COMPRESSED-SEQ-FILE]:spikenuc1107.fasta -z -i [INPUT-IDS] -f "[HEADER-FORMAT]" > [OUTPUT-SEQ-FILE]
```

If you like to see a progress bar, you cal include `-p` argument when running the tool.

Following is an example of running the tool on the example data.
```
bin/alignment-filter -a example/3/a.aln -i example/3/ids.txt -f "{1}|{2}|{0}" -p > out
```

Instead of filtering by ids, the fasta file could be filtered with names (the item starting with "hCov-19/..."). For that `-by name` should be added to the arguments.

If the items we want to be printed in the description of each sequence is not provided in the fasta file, it could be extracted from the metadata file. For this `-d metadata` should be added to the arguments.


## Build mutation samples files
Given a tree file and an alignment, we can infer internal node's genomic sequences. Then based on these sequences
we can create a file for each mutation representing samples with that specific mutation. This could be done for
DNA or protein sequences. 

## Ancestral Sequence Reconstruction (Inferring internal node's sequences)
As an example, we produce mutation-samples files for the example located in `example/1/` folder.

File `a.aln` represents the alignment, file `a.tree` represents the tree and file `cost.txt` contains
table of cost of mutations on the tree. 

First we infer internal nodes' sequences via following command
```
../../build/phylogeo_sankoff_general_dna --tree a.tree --aln a.aln --out mutation-samples.txt --cost cost.txt 
```
This command produces a file `mutation-samples.txt` containing mutations. Contents of this file would be
```
A       A2C_0
A       A3D_1
A       A4F_2
A       C7G_3
B       A2C_0
B       A3D_1
B       A4F_2
B       C7G_3
B       T9V_4
B       W10V_5
```
where the first column are samples and the second column represents a mutation name. Mutation name ends with '_' and a number. 
This ending part makes unique (potential) mutations that happens more than once on the tree.

## Creating separate files for each mutation
Then we create one file for each mutation. This could be done via following command
```
mkdir mutation-sample-separated/
../../build/mutation-samples --in mutation-samples.txt --out mutation-sample-separated/ --min 1
```
The file `mutation-samples.txt` is the output of the inference, folder `mutation-sample-separated/` will contain all the 
resulting files, and option `--min 1` tells `mutation-samples` to filter out mutations less than one samples. 
Pay attention to the '/' at the end of the output argument of the command. If you not include that, results would 
appear as separate files in the current folder with `mutation-sample-separated` as their prefix.
As the result the `mutation-sample-separated` folder will contain following files:
```
A2C_0.txt  A3D_1.txt  A4F_2.txt  C7G_3.txt  T9V_4.txt  W10V_5.txt
```
Each file contains name of samples as their lines.

The inference is done via Sankoff's algorithm.
