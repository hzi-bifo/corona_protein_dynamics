protein=$1
lineage_file=$3
aliases_file=$4

###
#get all all isolates that are in the subtree of an amino acid
python2 $2/map_substitutions_isolates.py ../output/$protein".summed_isolates.txt" \
	../data/$protein"_cds.map" ../$protein"_cluster_map.csv" positions/
#mv to folder
mkdir substitution_isolates
mv *_isolates.csv substitution_isolates/
#done
###

###
#get all all isolates that are in the subtree of an amino acid
python2 $2/map_pangolin_isolates.py $lineage_file $aliases_file
#mv to folder
mkdir pangolin_isolates
mv *_isolates.csv pangolin_isolates/
#done
###

###
#get overlap of pangolin isolates with substitution isolates
#very slow
cat all_lineages.csv  | while read -r lin; do
        f="pangolin_isolates/"$lin"_isolates.csv";
	echo -e " \t"$lin > $lin"_overlap.csv"; 
        cat mutation_list.csv | while read -r subs; do echo -e $subs"\t"$(awk 'FNR==NR{a[$1];next}($1 in a){print}' $f "substitution_isolates/"$subs"_isolates.csv" | wc -l); done  >> $lin"_overlap.csv" 
done


#mv to folder
mkdir overlap
mv *_overlap.csv overlap/
#done
###

###
#merge overlaps in one table
echo " " > tmp.txt && cat mutation_list.csv >> tmp.txt
for f in overlap/*_overlap.csv; do
#	t=$(basename $f)
#	echo ${t/_overlap.csv/} > tmp2.txt
	awk -F"\t" '{ print $2 }' $f > tmp2.txt
	paste tmp.txt tmp2.txt > tmp3.txt
	mv tmp3.txt tmp.txt
done
#done
###

#get size of pangolin lineages
for f in pangolin_isolates/*_isolates.csv; do name=$(basename $f); n=${name/_isolates.csv/}; b=$(wc -l $f | cut -d " " -f 1); echo -e  $n"\t"$b; done > lin_size.csv

#get size of pangolin lineages
for f in substitution_isolates/*_isolates.csv; do name=$(basename $f); n=${name/_isolates.csv/}; b=$(wc -l $f | cut -d " " -f 1); echo -e  $n"\t"$b; done > subs_size.csv


#filter each row and assign substitutions to pangolin lineages
python2 $2/pangolin_substitution_frequency.py tmp.txt lin_size.csv subs_size.csv subs_lin_mapping.csv
#rm tmp files
mv tmp.txt all_overlaps.csv
rm tmp*

###
#make R plot
#Rscript $2/make_heatmap.r heatmap_table.csv
#done
###

###
#mv files
#mv Rplots.pdf ../$protein"_heatmap.pdf"
#mv heatmap_table.csv ../$protein"_heatmap_table.csv"
#done
###
