import sys

meta_file = open(sys.argv[1])
meta_lines = meta_file.readlines()

ids = set([ line.replace("\n","") for line in sys.stdin])
meta_out_file = open("meta_data_filtered.tsv","w")
meta_out_file.write(meta_lines[0])
for meta_line in meta_lines[1:]:
	meta_id = meta_line.split("\t")[2] 
	if meta_id in ids:
		meta_out_file.write(meta_line)
meta_out_file.close()

