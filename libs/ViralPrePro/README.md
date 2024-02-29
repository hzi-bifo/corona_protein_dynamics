# ViralPrePro
This repository contains an automated pipeline for viral data preprocessing to subsequent phylogenetic analysis. It generates alignments of nucleotide and amino acid sequences and infers a phylogenetic tree for the nucleotide sequences.

In detail, the following modules are included:
- remove signal peptide (optional)
  - used for the HA protein of influenza to remove the signal peptide at the start of the sequence to initiate the correct numbering (PatchPipeline; numbering is manually adjusted later in SD plots pipeline)
- translate nucleotide sequences into amino acid sequences (optional)
  - can be used if only nucleotide sequences are available
- check for same cds and aa (optional)
  - checks whether the coding sequence has a corresponding amino-acid sequence with correct length (and whether the identifier are equal)
- sample a specified number of sequences per season (optional)
  - used for SweepDynamics plots to sample the same number of sequences for each season to avoid a bias
- initialize a consistent mapping
  - replaces the header of both cds and aa sequences with a new identifier to avoid errors regarding blanks
  - generates a mapping file which allows to map the original headers back to the multi fasta file
- collapse identical sequences (optional)
  - removes duplicate sequences, used for PatchPipeline when the frequency of the substitution does not affect the results
- generate alignments
  - creates multiple sequence alignments either without a reference (muscle) or with a reference (hmmalign)
- trim alignments (optional)
  - uses trimal to remove all positions in the alignments with gaps in at least 80% of the sequences with an minimum ammount of 50% of remaining gaps
- infer the phylogenetic tree
  - generates a phyologenetic tree with fasttree from the generated and curated sequence alignment of the coding sequence
  - Caution: Fasttree automatically roots the tree, which has no biological meaning at all. 
  - an additional script makes the tree binary 


NOTE: ideally, this script is called from another pipeline (e.g. PatchPipeline or AD/SD plots pipeline). In that case, you don't need to run the preprocessing separately as it will be done automatically. Look into the documentation of the respective pipeline to see how to run it.

If you want to run the preprocessing separately (e.g. if you only want to infer a tree), follow the instructions below.

Input:
Create one folder containing all files and use the foldername as infix for the following input files:
- a multi fasta file which contains amino acid sequences: \_aa.fasta (optional, can be inferred from coding sequences)
- a multi fasta file which contains corresponding coding sequences: \_cds.fasta

Configuration:
The parameters that need to be specified are listed in PrePro.conf. Edit the config file and run
> . PathTo/ViralPrePro/PrePro.conf

Running the pipeline:
> bash PathTo/ViralPrePro/viralPrePro.sh PathToInputFolder
