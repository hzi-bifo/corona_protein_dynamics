#!/bin/bash

# Function declarations

# Show help message
show_help() {
    echo "Usage: $0 -r <root_path> [-s <sample_size>] [-t <time_period>] [-g <location>] -c <genomes_file> -l <lineage_file>"
    echo "Options:"
    echo "  -r          Path to the root"
    echo "  -s          Number of sampled sequences (if not set, no sampling)"
    echo "  -t          Analyze by given time period (year, month, day, season)"
    echo "  -g          Sample per geographic location (continent, country). If fasta is from one location, no need to specify"
    echo "  -c          Get coding sequences first (specify genomes file)"
    echo "  -l          Map substitutions to pangolin lineages (specify lineage file)"
    # echo "  -w, --whole      Infer whole-genome tree (specify genomes file)"
    # echo "  -p, --patch      Generate input for subsequent patch analysis"
    echo "  -h, --help  Show help"
}

# Validate required arguments
validate_args() {
    if [ -z "$root_path" ]; then
        echo "Error: root path is required."
        exit 1
    else
        if [ ! -f "$root_path" ]; then
            echo "Error: root path does not exist."
            exit 1
        fi

    fi
    # if [ -z "$time_period" ]; then
    #     echo "Error: time period is required."
    #     exit 1
    # fi
    # if [ -z "$location" ]; then
    #     echo "Error: location is required."
    #     exit 1
    # fi
    local valid_locations=("continent" "country")
    if [ -n "$location" ]; then
        if [[ ! " ${valid_locations[@]} " =~ " $location " ]]; then
            echo "Error: Invalid location. Please use: continent or country"
            exit 1
        fi
    fi

    local valid_time_periods=("year" "month" "day" "season")
    if [ -n "$time_period" ]; then
        if [[ ! " ${valid_time_periods[@]} " =~ " $time_period " ]]; then
            echo "Error: Invalid time period. Please use: year, month, day, or season"
            exit 1
        fi
    fi

    if [ -n "$sample_size" ] && ! [[ "$sample_size" =~ ^[0-9]+$ ]]; then
        echo "Error: Sample size must be an integer."
        exit 1
    fi

    if [ -n "$genomes_file" ] && [ ! -f "$genomes_file" ]; then
        echo "Error: Genomes file does not exist."
        exit 1
    fi

    if [ -n "$lineage_file" ] && [ ! -f "$lineage_file" ]; then
        echo "Error: Lineage file does not exist."
        exit 1
    fi
}

# Main script logic
main() {
    # Set up environment variables
    proj_dir=$(realpath "$(dirname "$0")")
    

    libs_path="${proj_dir}/libs"
    python_tools_path="${proj_dir}/software"
    viralprepro_pipeline="${libs_path}/ViralPrePro/viralPrePro.sh"
    sankoff_path="${libs_path}/phylogeo-tools/build"
    cost_matrix="$proj_dir/cost_matrix/cost_matrix.txt"

    # Initialize variables
    root_path=""
    sample_size=0
    time_period="month"
    location=""
    get_coding_sequences=false
    map_lineages=false
    # whole_genome=false
    # run_patches=false
    genomes_file=""
    lineage_file=""

    # Check for help option before processing other arguments
    for arg in "$@"; do
        if [[ "$arg" == "-h" || "$arg" == "--help" ]]; then
            show_help
            exit 0
        fi
    done

    # Capture the first positional argument as output directory
    if [ $# -lt 1 ]; then
        # echo "Error: Output directory is required."
        # exit 1
        show_help
        exit 0
    fi

    # if arugument is not a directory
    if [ ! -d "$1" ]; then
        echo "Please specify an output directory as the first argument."
        show_help
        exit 1
    fi

    # output_dir="$1"
    output_dir=$(realpath "$1")
    shift  # Shift the positional parameters to the left.


    # Parse arguments
    while getopts ":r:s:t:g:c:l:h" opt; do
        case $opt in
            r) [ -n "$OPTARG" ] && root_path=$(realpath "$OPTARG") ;;
            s) sample_size="$OPTARG" ;;
            t) time_period="$OPTARG" ;;
            g) location="$OPTARG" ;;
            c) [ -n "$OPTARG" ] && get_coding_sequences=true && genomes_file=$(realpath "$OPTARG") ;;
            l) [ -n "$OPTARG" ] && map_lineages=true && lineage_file=$(realpath "$OPTARG") ;;
            h) show_help
                exit 0 ;;
            \?) echo "Invalid option: -$OPTARG" >&2
                show_help
                exit 1 ;;
        esac
    done
    shift $((OPTIND -1))

    echo "Running protein mutation dynamics analysis with metadata $lineage_file for $genomes_file"

    # Validate arguments
    validate_args

    # Your script's main logic starts here...

    # Check if fasta header is correct
    if [ "$get_coding_sequences" = true ]; then
        valid_genome_header=$(grep '>' "$genomes_file" |\
            awk -F"|" '{ if ($2 == "" || $3 == "" ) print "false" }')
        if [ -n "$valid_genome_header" ]; then
            echo "The script terminates because the fasta file header are not correct."
            echo "Please use fasta header as follows: >hCoV-19/Wuhan/IPBCAMS-WH-01/2019|EPI_ISL_402123|2019-12-24"
            exit 1
        fi
    fi

    # change to the output directory
    # output_dir=$(realpath $1)
    cd "$output_dir" || exit 1
    protein=$(basename "$output_dir")

    echo "Starting pipeline in working directory: $output_dir"

    # Process coding sequences or whole-genome tree
    if [ "$get_coding_sequences" = true ]; then
        python2 "$python_tools_path/orf_finder.py" "$genomes_file" "${protein}_complete.fasta"
    fi

    echo "Extracted coding sequences in ${protein}_complete.fasta"

    # Check if cds fasta header is correct
    if [ ! -f "${protein}_complete.fasta"  ]; then
        echo "The input file is not available, expecting ${protein}_complete.fasta."
        exit 1
    else
        valid_protein_header=$(grep '>' "${protein}_complete.fasta" |\
            awk -F"|" '{ if ($2 == "" || $3 == "" ) print "false" }')
        if [ -n "$valid_protein_header" ]; then
            echo "The script terminates because the cds fasta file header are not correct."
            echo "Please use fasta header as follows: >hCoV-19/Wuhan/IPBCAMS-WH-01/2019|EPI_ISL_402123|2019-12-24"
            exit 1
        fi
    fi

    root=$(grep '>' "${root_path}" | awk -F"|" '{ print substr($1,2); }')
    ##add the root if it is missing
    #todo why the root appears twice in the the final output
    in=$(grep "$root" "${protein}_complete.fasta")
    if [ -z "$in" ]
    then 
        cat "${root_path}" >> "${protein}_complete.fasta"
    fi

    cmd=""
    if [[ -n $time_period ]] ; then
        cmd="${cmd} -t ${time_period}"
    fi
    if [[ -n $location ]] ; then
        cmd="${cmd} -g ${location}"
    fi

    if [ $sample_size -eq 0 ] ; then
        echo "No sampling is done in ${protein}_complete.fasta only splitting by time periods"
    else
        echo "Starting sampling cds sequences in ${protein}_complete.fasta"
    fi
    
    #omit incorrect time stamps and sample sequences per period
    # when sample_size is 0, no sampling is done
    python2 "$python_tools_path/sampling/sampling.py" -f "${protein}_complete.fasta" \
        -s $sample_size -r "$root" $cmd

    echo "Removing duplicates in cds sequences"
    #rearrange folder
    shopt -s nullglob
    mkdir -p time_periods
    for f in sequence*.fa; do
        name=${f/.fa/}
        mkdir "$name"
        mv "$f" "${name}/"
        cd "$name" || exit 1
        ###
        #cluster sequences per time period 
        if true ; then 
            ###
            #cd-hit remove duplicates
            cd-hit-est -i "$f" -o "$name" -c 1 -d 0 > /dev/null
            cp "$f" "${name}_old.fa"
            mv "$name" "$f"
            ###
            #add weigth to header
            python2 "${python_tools_path}/cluster_weight.py" "${name}.clstr" "$f" > test1.txt
            mv test1.txt "$f"
            ###
            #merge clusters
            python2 "${python_tools_path}/merge_cluster.py" "${name}.clstr" > "${name}_cluster_map.csv"
        else
            ###
            #seqkit
            seqkit rmdup "$f" -s -o "${name}_tmp.fa" -D "${name}.csv" 2> /dev/null
            mv "$f" "${name}_old.fa"
            if [ ! -f "${name}.csv" ]; then
                cat "${name}_tmp.fa" | while read -r line; do if [[ $line == ">"* ]]; \
                    then echo "${line}|weight=1"; else echo "$line"; fi; done > "$f"
            else
                cat "${name}_tmp.fa" | while read -r line; do if [[ $line == ">"* ]]; \
                    then echo -n "$line" ;grep "${line:1}" "${name}.csv" |\
                    awk '{print "|weight="$1} END { if (!NR) print "|weight=1" }'; else echo "$line"; fi; done > "$f"
            fi
        fi
        ###
        #merge all representative sequences from cluster in one file
        cat "$f" >> "../${protein}_cds_old_header.fa"
        cd ..
        ###
        #clean up and mv all sequence folder aside
        mv "$name" time_periods/
    done
    cat time_periods/*/*_cluster_map.csv > $protein"_cluster_map.csv"
    if [ "$time_period" = "season" ]; then
        mv "${protein}_cds_old_header.fa" "${protein}_cds_all.fa"
    else
        ###
        #adjust header hereafter by replacing time periods
        python2 "${python_tools_path}/adjust_header.py" "${protein}_cds_old_header.fa" \
            "${protein}_cds_all.fa" "$time_period" "$protein" && rm "${protein}_cds_old_header.fa"
    fi


    ###
    #convert to amino acids
    echo "Converting to amino acids..."
    python2 "${python_tools_path}/cdsTranslator.py" "${protein}_cds_all.fa" > "${protein}_aa_all.fa"

    ###
    #adapt header for SD plots, the output files are "${protein}_{cds,aa}.fa"
    python2 "${python_tools_path}/new_header.py" "${protein}_cds_all.fa" "$protein" && \
        python2 "${python_tools_path}/new_header.py" "${protein}_aa_all.fa" "$protein" 
    rm "${protein}_cds_all.fa" "${protein}_aa_all.fa"

    ###
    #get sequence stats
    #make time file for period-wise plotting
    python "${python_tools_path}/sequence_stats.py" "${protein}_complete.fasta" "$time_period"
    Rscript "${python_tools_path}/plot_seq_stats.r" "seq_counts_${protein}_complete.csv"
    awk 'NR>2 {if ($2>0) print $1}' seq_count* > "${protein}_time_map.csv"

    ##################################
    ###data preperation done until here
    ###################################

    ###
    #run the viral prepro to infer alignment and tree
    echo "Running ViralPrePro..."
    # export name=$protein
    nameViralPreProPath="$(dirname "${viralprepro_pipeline}")/"
    export nameViralPreProPath
    export refalign=true
    export checkSameSeq=true
    bash "$viralprepro_pipeline" "$output_dir"
    #this runs the new sd plots
    bash "${python_tools_path}/SD_tools/make_SD_plot.sh" "$output_dir" "${python_tools_path}/SD_tools" \
        "$sankoff_path" "$cost_matrix" &
    pid=$!
    while kill -0 $pid 2> /dev/null; do
        echo -ne "Generate SD plot.    \r"
        sleep 1
        echo -ne "Generate SD plot..   \r"
        sleep 1
        echo -ne "Generate SD plot...  \r"
        sleep 1
    done

    # move unsampled, sampled, mapped and aligned sequences into one folder
    mkdir -p data
    mv filtered_seq.txt ./*.fa ./*.aln ./*.map ./*.phy ./*.tre -t data/ #2> /dev/null

    # move data from sd plots in to output folder
    mkdir -p output
    mv "${protein}.results."* "${protein}.numIsolates.txt" "${protein}.mutations.per_position.txt" \
        "${protein}.s"*.txt ./*.json -t output/ 2> /dev/null

    ###
    #get recurrent subsitutions
    a=$(grep -v ">" "$root_path" | wc -m)
    length=$((a/3))
    mkdir -p recurrentSubstitutions
    cd recurrentSubstitutions || exit 1
    echo -e "Residue\tcounts" > "${protein}_recurrent_substitutions.csv" && \
        awk '{ print $1 }' ../output/*.results.frequencies.txt | sed 's/[A-Z\-]//g' |\
        uniq -c | awk '{ if ($1 != 1) print $2"\t"$1}' >> "${protein}_recurrent_substitutions.csv"
    awk '{ print $1 }' ../output/*.results.frequencies.txt | sed 's/[A-Z\-]//g' |\
        uniq -c | awk '{print $2"\t"$1}' > tmp.csv
    in=$(tail -n 1 tmp.csv | grep $length)
    if [ -z "$in" ]
    then
        echo -e $length"\t0" >> tmp.csv
    fi
    echo -e "Residue\tcounts" > "${protein}_substitutions.csv" && \
        awk '{while (++i<$1) print i"\t0"}1' tmp.csv >> "${protein}_substitutions.csv"
    rm tmp.csv
    echo "$protein" > "${protein}_data_size.csv"
    grep -o "EPI" "../${protein}_cluster_map.csv" | wc -l >> "${protein}_data_size.csv"
    cd ../
    ###
    #make pngs from pdfs
    for file in *.pdf;
    do
        pngfile=${file/.pdf/.png}
        convert -density 300 "$file" "$pngfile"
    done
    # exit

    # ###
    # #map lineages
    if [ "$map_lineages" = true ]; then
        cd "${output_dir}/output" || exit 1
        Rscript "${python_tools_path}/SD_tools/mutationmaps.r" "${protein}"
        cd ..

        python "${python_tools_path}/mutation_lineage.py" "output/${protein}.mapping_header.json" \
            "output/${protein}.cluster_map.json" "${protein}_sankoff_outfile.txt" "$lineage_file" \
            "${protein}.mutation_lineage"
        
        python "${python_tools_path}/mutation_lineage_json.py" "${protein}.mutation_lineage.summary.tsv" \
            "${protein}.mutation_lineage.summary.json"
    fi

    # More of your script's main logic here...
    echo "Script execution completed."
    exit 0
}

# Call the main function with all script arguments
main "$@"
