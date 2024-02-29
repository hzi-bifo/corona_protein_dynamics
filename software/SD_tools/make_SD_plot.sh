
cd $1
protein=$(basename $1)
SoftwarePath=$2
SANKOFF_PATH=$3
cost_matrix=$4


####
#run sankoff
sankoff_out=$protein"_sankoff_outfile.txt"
mutation_folder="mutation-sample-separated/"

echo "----- 2. Run Sankoff -----"
$SANKOFF_PATH/phylogeo_sankoff_general_dna --tree "$protein"_cds_binary.phy --aln "$protein"_aa_mapped_c.aln --cost $cost_matrix --out "$sankoff_out" --omit-leaf-mutations #2> /dev/null
mkdir -p "$mutation_folder"
rsync -a --delete "$mutation_folder" 2> /dev/null
$SANKOFF_PATH/mutation-samples --in "$sankoff_out" --out "$mutation_folder" --min 2


###
#parse header data to get dates to json
metafile="metadata.json"
if [ ! -s $protein"."$metafile ]; then
	grep '>' $protein"_complete.fasta" > $protein"_complete_header.txt"
	Rscript $SoftwarePath/parse_dates.R $protein"_complete_header.txt" $protein"."$metafile
fi


###
#parse header mapping file to json
mappingfile="mapping_header.json"
if [ ! -s $protein"."$mappingfile ]; then
	python $SoftwarePath/parse_mappingfile.py $protein"_cds.map" $protein"."$mappingfile
fi

###
#parse cluster_map.csv file and save dictionary to json
clusterfile="cluster_map.json"
if [ ! -s $protein"."$clusterfile ]; then
	python $SoftwarePath/parse_cluster.py $protein"_cluster_map.csv" $protein"."$clusterfile
fi


cd $mutation_folder
START=1
END=$(ls -l | awk '{ print $9 }' | awk -F"_" '{ print $1 }' | sed 's/[A-Z]//g' | sort -n | tail -n 1)
###
#add isolates from clusters
for i in $(seq 1 $END); do
	#echo $i
	for f in [A-Z\-]$i[A-Z\-]*.txt; do
		if [ -f $f  ]; then
			if [ -z "$(grep -- 'EPI' $f)" ]; then
				python $SoftwarePath/map_header.py $f ../$protein"."$mappingfile
			fi
			python $SoftwarePath/map_cluster_seq.py $f ../$protein"."$clusterfile
		fi
	done
done
###
#calculate frequency for each position
for i in $(seq $START $END); do
	files=$(ls | grep [A-Z\-]$i[A-Z\-])
	if [ ! -z "$files" ]; then
		#echo $i
		#echo $files
		echo $files | python $SoftwarePath/calc_pos_freq.py ../$protein"."$metafile ../$protein"_time_map.csv" \
			../$protein".numIsolates.txt" ../$protein".subtreeMutationMap.per_position.txt" \
			../$protein".mutations.per_position.txt" ../$protein"."$mappingfile
	fi
done
cd ..

#add line break to subtreemutationmap
echo >> $protein.mutations.per_position.txt


###
#make num isolate file for plotting 
timefile="timedata.json"
grep '>' "$protein"_cds.fa | awk '{ print $9" "$13 }' | python $SoftwarePath/make_numIsolates.py \
	"$protein".numIsolates.txt


threshold=0.1
format="pdf"
filtersign="false"
Rscript $SoftwarePath/fishertest.r $protein $threshold
echo -e "support\tleafs\tsubstitution" >> $protein".support_values.txt"

Rscript $SoftwarePath/buildAD_sign_monthly.r $protein $threshold $SoftwarePath $filtersign 12 $format 0 #> /dev/null 
if [ ! -s $protein".results.txt" ]; then
	echo "No significant changes have been detected. A plot will be created showing all non-significant changes."
else
	mv $protein"."$format $protein".significant_positions."$format
fi
