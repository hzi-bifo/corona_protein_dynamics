import json
import csv
import argparse

def read_json(filename):
    with open(filename, 'r') as file:
        return json.load(file)

def process_files(mapping_header_file, cluster_map_file, sankoff_file, metadata_file, output_prefix):
    # Read JSON files
    # cluster: representative sequence
    mapping_header = read_json(mapping_header_file)
    # representative sequence: other sequences in cluster
    cluster_map = read_json(cluster_map_file)

    # Process Sankoff output
    sankoff_data = {}
    with open(sankoff_file, 'r') as file:
        for line in file:
            cluster, mutation = line.strip().split('\t')
            mutation = mutation.rsplit("_", 1)[0]
            sankoff_data[cluster] = mutation

    # Process metadata
    metadata = {}
    with open(metadata_file, 'r') as file:
        reader = csv.reader(file, delimiter='\t')
        for row in reader:
            metadata[row[2]] = row[18]  # Assuming sequence ID is in the 3rd column and lineage in the 19th

    output_individual_file = output_prefix + '.invididual.tsv'
    output_summary_file = output_prefix + '.summary.tsv'
    
    mutation_lineage_dict = {}
    # {mutation: {lineage: count}}
    # Write output
    with open(output_individual_file, 'w') as out:
        writer = csv.writer(out, delimiter='\t')
        writer.writerow(['cluster', 'sequenceID', 'lineage', 'mutation', 'sig'])
        
        

        for cluster, rep_seq_id in mapping_header.items():
            mutation = sankoff_data.get(cluster, '')
            if mutation:
                sig = '*' # if mutation else ''
                lineage = metadata.get(rep_seq_id, '')
                writer.writerow([cluster, rep_seq_id, lineage, mutation, sig])
                if mutation in mutation_lineage_dict:
                    if lineage in mutation_lineage_dict[mutation]:
                        mutation_lineage_dict[mutation][lineage] += 1
                    else:
                        mutation_lineage_dict[mutation][lineage] = 1
                else:
                    mutation_lineage_dict[mutation] = {lineage: 1}

                # Write other members of the cluster
                for member in cluster_map.get(rep_seq_id, []):
                    lineage = metadata.get(member, '')
                    writer.writerow([cluster, member, lineage, mutation, sig])
                    if mutation in mutation_lineage_dict:
                        if lineage in mutation_lineage_dict[mutation]:
                            mutation_lineage_dict[mutation][lineage] += 1
                        else:
                            mutation_lineage_dict[mutation][lineage] = 1
                    else:
                        mutation_lineage_dict[mutation] = {lineage: 1}
        
    with open(output_summary_file, 'w') as out:
        writer = csv.writer(out, delimiter='\t')
        writer.writerow(['mutation', 'lineage_frequency', 'count'])
        for mutation, lineage_count in mutation_lineage_dict.items():
            total_count = sum(lineage_count.values())
            sorted_lineage = sorted(lineage_count, key=lineage_count.get, reverse=True)

            lineage_frequency = ", ".join([lineage+":"+str(round(lineage_count[lineage]/float(total_count), 3)) for 
                                        lineage in sorted_lineage])
            writer.writerow([mutation, lineage_frequency, total_count])
        

def main():
    parser = argparse.ArgumentParser(description="Generate mutation lineage mapping file.")
    parser.add_argument("mapping_header", help="JSON file with sequence clusters and representative sequence IDs")
    parser.add_argument("cluster_map", help="JSON file with representative sequence IDs and other members of each cluster")
    parser.add_argument("sankoff_outfile", help="Text file with cluster names and mutations")
    parser.add_argument("metadata", help="TSV file with sequence ID and lineage information")
    parser.add_argument("output", help="Output prefix for TSV files")

    args = parser.parse_args()

    process_files(args.mapping_header, args.cluster_map, args.sankoff_outfile, args.metadata, args.output)

if __name__ == "__main__":
    main()
