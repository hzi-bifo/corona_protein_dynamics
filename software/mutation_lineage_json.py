import csv
import json
import argparse

def process_file(input_file, output_file):
    result = []

    with open(input_file, 'r') as file:
        reader = csv.DictReader(file, delimiter='\t')
        for row in reader:
            # Format lineage_frequency
            lineage_frequency = row['lineage_frequency'].replace(':', ' (') + ')'
            lineage_frequency = lineage_frequency.replace(', ', '), ')
            
            # Create a dictionary for each row
            row_dict = {
                "mutation": row['mutation'],
                "lineages": lineage_frequency,
                "count": row['count']
            }
            result.append(row_dict)

    # Write to JSON file
    with open(output_file, 'w') as outfile:
        json.dump(result, outfile, indent=4)

def main():
    parser = argparse.ArgumentParser(description="Convert a mutation lineage TSV file to a JSON format.")
    parser.add_argument("input_file", help="Input TSV file with mutation data")
    parser.add_argument("output_file", help="Output JSON file")

    args = parser.parse_args()
    process_file(args.input_file, args.output_file)

if __name__ == "__main__":
    main()
