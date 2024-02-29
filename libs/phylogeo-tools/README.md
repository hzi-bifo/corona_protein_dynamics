# Tools for Phylogeography analysis of the trees and related data

# Build
to build, just run 
```
make
```
at the root. 
Note that there are some parameters that should be setted correctly in the makefile. One is `CONDA_PREFIX` which
is used in compilation of some applications.

# Build mutation samples files
Given a tree file and an alignment, we can infer internal node's genomic sequences. Then based on these sequences
we can create a file for each mutation representing samples with that specific mutation. This could be done for
DNA or protein sequences. 

## Inferring internal node's sequences
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