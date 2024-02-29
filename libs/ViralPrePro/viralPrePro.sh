cd $1
#####

nameInOutFolder=$1"/"
name=$(basename $nameInOutFolder)

SCRIPTPATH_VPP="${nameViralPreProPath}software"

# remove signal peptide
echo " ----- 1.1 Remove signal peptide -----"

if [ "$removeSignalPeptide" = true ]
then
	# overwrite fasta file if $name"_original_cds.fa does not  exist
	if [ ! -s $nameInOutFolder$name"_original_cds.fa" ]
	then
		cp $name"_cds.fa"  $name"_original_cds.fa"
	fi
	# remove signal peptide
	python $SCRIPTPATH_VPP'/removeSignalPeptide.py' $name"_original_cds.fa" 'cds' $rSP_Subtype > $name"_cds.fa"

	if [ "$cdsTrans" = false ]
	then
		# overwrite fasta file if $name"_original_cds.fa does not  exist
        if [ ! -s $nameInOutFolder$name"_original_aa.fa" ]
		then
        	cp  $name"_aa.fa"  $name"_original_aa.fa"
        fi
		# remove signal peptide
		python $SCRIPTPATH_VPP'/removeSignalPeptide.py' $name"_original_aa.fa" 'aas' $rSP_Subtype > $name"_aa.fa"
	fi

	echo "Done!"
	echo
else
	echo "Omitted."
	echo
fi

# translate cds into aa
echo " ----- 1.2 Translate cds into aa -----"

if [ "$cdsTrans" = true ]
then
	if [ "$correctFrame" = true ]
	then
		python $SCRIPTPATH_VPP/cdsTranslator.py $name$"_cds.fa" > $name$"_aa.fa"
	else
		# translate into all 3 forward and 3 reverse frames
		perl $SCRIPTPATH_VPP/translate_all.pl $name$"_cds.fa" > $name$"_aa_allframes.fa"
		# blast against reference sequence
		# $SCRIPTPATH_VPP/blastp -query $name$"_aa_allframes.fa" -subject $aa_ref -outfmt 6 -evalue 0.0001 | awk '{ print $1 }' > correct_translation.txt
		blastp -query $name$"_aa_allframes.fa" -subject $aa_ref -outfmt 6 -evalue 0.0001 | awk '{ print $1 }' > correct_translation.txt
		# use frame aligning to reference as the correct translation
		perl $SCRIPTPATH_VPP/get_correct_frame.pl $name$"_aa_allframes.fa" correct_translation.txt $name$"_aa.fa"
		# remove temporary files
		rm $name$"_aa_allframes.fa" correct_translation.txt
	fi
	echo "Done!"
	echo
else
	echo "Omitted."
	echo
fi

####
#0 check for same cds and aa
####
echo " ----- 1.3 Checking for same cds and aa -----"

if [ "$checkSameSeq" = true ]
then
	python $SCRIPTPATH_VPP/check_cds_aa.py $name$"_cds.fa" $name$"_aa.fa" > filtered_seq.txt      #2>>$PWDPATH/'log.txt'
	f=($name$"_cds_purged.fa")
	if [ -f $f ]
	then
		mv $name$"_cds.fa" $name$"_cds_original.fa"
		mv $name$"_cds_purged.fa" $name$"_cds.fa"
	fi

	f=($name$"_aa_purged.fa")
	if [ -f $f ]
	then
		mv $name$"_aa.fa" $name$"_aa_original.fa"
			mv $name$"_aa_purged.fa" $name$"_aa.fa"
	fi
	echo "Done!"
	echo
else
	echo "Omitted."
	echo
fi

###
#0 sample a specified number of sequences per season
###
echo " ----- 1.4 Sample data -----"

if [ "$sample_season" = true ]
then
	f=($name"_aa_notsampled.fa")
	if [ ! -f $f ]
	then
		#sample data
		perl $SCRIPTPATH_VPP/sample_season.pl $name $sample_size "$rootseq"

		if [ $? -eq 2 ]; then
			# exit code 2 of sample_season.pl: root sequence not found
			exit
		fi
		#rename files
		mv $name"_aa.fa" $name"_aa_notsampled.fa"
		mv $name"_cds.fa" $name"_cds_notsampled.fa"
		mv $name"_aa_sampled.fa" $name"_aa.fa"
		mv $name"_cds_sampled.fa" $name"_cds.fa"
	fi
	echo "Done!"
	echo
else
	echo "Omitted."
	echo
fi
#exit
####
#1 initialize a consistent mapping for the DnDs pipeline
####
echo " ----- 1.5 Initialize consistent mapping -----"

f=($name$"_cds_mapped.fa")
if [ ! -f $f ]
then
	perl $SCRIPTPATH_VPP/mapping.pl $name
fi

echo "Done!"
echo

####
#1 collapse identical sequences
#### 
echo " ----- 1.6 Collapse identical sequences -----"

if [ "$collapseSeq" = "true" ]
then
    python $SCRIPTPATH_VPP/removeIdentical.py $name$"_cds_mapped.fa" $name$"_aa_mapped.fa" $name$"_cds_mapped.fa" $name$"_aa_mapped.fa"
	echo "Done!"
	echo
else
	echo "Omitted."
	echo
fi


####
#2 generate alignments
####
echo " ----- 1.7 Generate alignments -----"
### for protein sequences
f=($name$"_aa_mapped.aln")
if [ ! -f $f ] # if alignment for aa does not exist
then
	if [ "$refalign" = true ]
	then
		#first extract root sequence, it is the first entry in file
		awk "/^>/ {n++} n>1 {exit} {print}" $name$"_aa_mapped.fa"  > root_sequence_aa.fa
		mafft --quiet --6merpair --keeplength --addfragments $name$"_aa_mapped.fa" root_sequence_aa.fa > tmp_aa.aln &
		pid=$!
		while kill -0 $pid 2> /dev/null; do
			echo -ne "Generate amino acid alignment.    \r"
			sleep 1
			echo -ne "Generate amino acid alignment..   \r"
			sleep 1
			echo -ne "Generate amino acid alignment...  \r"
			sleep 1
		done
		#first extract root sequence, it is the first entry in file
		awk "/^>/ {n++} n>1 {exit} {print}" $name$"_cds_mapped.fa"  > root_sequence_cds.fa
		mafft --quiet --6merpair --keeplength --addfragments $name$"_cds_mapped.fa" root_sequence_cds.fa > tmp_cds.aln &
		pid=$!
		while kill -0 $pid 2> /dev/null; do
			echo -ne "Generate amino acid alignment.    \r"
			sleep 1
			echo -ne "Generate amino acid alignment..   \r"
			sleep 1
			echo -ne "Generate amino acid alignment...  \r"
			sleep 1
		done
		###
		#remove reference sequence from alignment
		#this removes everythin until the second occurrence of > 
		awk -v 'n=2' '/>/ && !--n, 0' tmp_aa.aln > $name$"_aa_mapped.aln"
		awk -v 'n=2' '/>/ && !--n, 0' tmp_cds.aln > $name$"_cds_mapped.aln"
	else
                mafft --amino --quiet $name$"_aa_mapped.fa" > $name$"_aa_mapped.aln" &
		pid=$!
		while kill -0 $pid 2> /dev/null; do
			echo -ne "Generate amino acid alignment.    \r"
			sleep 1
			echo -ne "Generate amino acid alignment..   \r"
			sleep 1
			echo -ne "Generate amino acid alignment...  \r"
			sleep 1
		done
		perl $SCRIPTPATH_VPP/pal2nal.pl $name$"_aa_mapped.aln" $name$"_cds_mapped.fa" -output "fasta" > $name$"_cds_mapped.aln" &
        	pid=$!
		while kill -0 $pid 2> /dev/null; do
			echo -ne "Generate nucleotide alignment.    \r"
			sleep 1
			echo -ne "Generate nucleotide alignment..   \r"
			sleep 1
			echo -ne "Generate nucleotide alignment...  \r"
			sleep 1
		done
	fi
fi
echo "Done!                           "
echo
####
#3 curate the alignments with trimal
####
echo " ----- 1.8 Curate alignments -----"
if [ "$refalign" = true ]
then
	echo "No trimal."
	cp $name$"_cds_mapped.aln" $name$"_cds_mapped_c.aln"
	cp $name$"_aa_mapped.aln" $name$"_aa_mapped_c.aln"
	echo "Omitted."
	echo
else
	if [ "$trimAl" = true ]
	then
		f=($name$"_cds_mapped_c.aln")
		####
		#TODO:
		#add comment why awk is used
		if [ ! -f $f ]
		then
		        # $SCRIPTPATH_VPP/trimal -in $name$"_cds_mapped.aln" -fasta -gt 0.2 -cons 50 | awk '{ print $1 }'  > $name$"_cds_mapped_c.aln"     #2>>$PWDPATH/'log.txt'
		        trimal -in $name$"_cds_mapped.aln" -fasta -gt 0.2 -cons 50 | awk '{ print $1 }'  > $name$"_cds_mapped_c.aln"     #2>>$PWDPATH/'log.txt'
		        #-gt 0.2 removes all pos in the alignment with gaps in 80% of the sequences
			        #-cons 50, at least 50% of all position are not remove 
		fi
	
		f=($name$"_aa_mapped_c.aln")
		if [ ! -f $f ]
		then
		        # $SCRIPTPATH_VPP/trimal -in $name$"_aa_mapped.aln" -fasta -gt 0.2 -cons 50 | awk '{ print $1 }'  > $name$"_aa_mapped_c.aln"       #2>>$PWDPATH/'log.txt'
		        trimal -in $name$"_aa_mapped.aln" -fasta -gt 0.2 -cons 50 | awk '{ print $1 }'  > $name$"_aa_mapped_c.aln"       #2>>$PWDPATH/'log.txt'
		fi
		echo "Done!"
	        echo
	else
		cp $name$"_cds_mapped.aln" $name$"_cds_mapped_c.aln"
		cp $name$"_aa_mapped.aln" $name$"_aa_mapped_c.aln"
		echo "Omitted."
		echo
	fi
fi


######
#here, the cds and aa analysis is completed
#we use now the whole genome file (as cds file) to infer the tree
#afterwards, reset all naming tocds files and use the whole genoome tree
if [ "$whole_genome_tree" = "true" ]
then
	if [ ! -f "whole_genome_mapped.aln" ]
	then
		echo " ----- 1.9 Infer whole genome phylogenetic tree -----"
		#adapt mapping from cds file to wg file
		python $SCRIPTPATH_VPP/whole_genome_header.py $name"_cds.map" $genomes_file > "whole_genome_mapped.fa"
		#name cds to tmp name
		mv $name$"_cds_mapped.aln" $name$"_cds_mapped_tmp.aln" 
		#make wg alignment and name it as cds alignment
		mafft --quiet "whole_genome_mapped.fa" > $name$"_cds_mapped.aln" &
		pid=$!
	        while kill -0 $pid 2> /dev/null; do
	        	echo -ne "Generate whole-genome alignment.    \r"
	        	sleep 1
	        	echo -ne "Generate whole-genome alignment..   \r"
	        	sleep 1
	        	echo -ne "Generate whole-genome alignment...  \r"
			sleep 1
		done
	        #infer tree on wg file (cds name)
		# $SCRIPTPATH_VPP/fasttree -gtr -quiet -nt $name$"_cds_mapped_c.aln" > $name$"_cds.phy" 2> /dev/null &                 #2>>$PWDPATH/'log.txt'
		fasttree -gtr -quiet -nt $name$"_cds_mapped_c.aln" > $name$"_cds.phy" 2> /dev/null &                 #2>>$PWDPATH/'log.txt'
	        pid=$!
	        while kill -0 $pid 2> /dev/null; do
		        echo -ne "Infer whole-genome tree.    \r"
	                sleep 1
	                echo -ne "Infer whole-genome tree..   \r"
	                sleep 1
	                echo -ne "Infer whole-genome tree...  \r"
	                sleep 1
	             done
	        #make the tree binary   
	        python $SCRIPTPATH_VPP/resolve_multi.py -i $name$"_cds.phy" -o $name$"_cds_binary.phy"      #2>>$PWDPATH/'log.txt'
		#name wg alignment
		#name cds correct
		mv $name$"_cds_mapped.aln" "whole_genome_mapped.aln"
		mv $name$"_cds_mapped_tmp.aln" $name$"_cds_mapped.aln"
	fi
fi

####
#4 infer the phylogenetic tree with fasttree based on the the coding sequences
####

f=(*.phy)
if [ ! -f $f ]
then
	echo " ----- 1.9 Infer cds phylogenetic tree -----"
        # $SCRIPTPATH_VPP/fasttree -gtr -quiet -nt $name$"_cds_mapped_c.aln" > $name$"_cds.phy" 2> /dev/null &                 #2>>$PWDPATH/'log.txt'
        fasttree -gtr -quiet -nt $name$"_cds_mapped_c.aln" > $name$"_cds.phy" 2> /dev/null &                 #2>>$PWDPATH/'log.txt'
        pid=$!
        while kill -0 $pid 2> /dev/null; do
	        echo -ne "Infer tree.    \r"
                sleep 1
                echo -ne "Infer tree..   \r"
                sleep 1
                echo -ne "Infer tree...  \r"
                sleep 1
             done
        
        # reroot the tree (root is first sequence in file, with id f0dp0)
        Rscript $SCRIPTPATH_VPP/reroot_tree.R $name$"_cds.phy" "f0dp0"

	#make the tree binary   
        python $SCRIPTPATH_VPP/resolve_multi.py -i $name$"_cds.phy" -o $name$"_cds_binary.phy"      #2>>$PWDPATH/'log.txt'
	echo "Done!"
fi
echo
