
../../build/phylogeo_sankoff_general_dna --tree S_Germany_cds_binary.phy --aln S_Germany_aa_mapped_c.aln --out mutation-samples.txt --cost cost.txt 
mkdir -p mutation-sample-separated/
rsync -a --delete mutation-sample-separated/

../../build/mutation-samples --in mutation-samples.txt --out mutation-sample-separated/ --min 2
