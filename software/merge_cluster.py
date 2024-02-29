import sys

cluster_file = open(sys.argv[1])
cluster_line = cluster_file.readlines()
out_line = ""
for i in range(len(cluster_line)):
	line = cluster_line[i]
	if i == 0:
		rep = ""
		out_line = ""
	elif line.startswith(">"):
		rep = ""
		print out_line[:-1]
		out_line = ""
	else:
		epi_id = line.split("|")[1]
		if rep == "":
			rep = epi_id
			out_line = out_line + "%s\t"%(epi_id)
		else:
			out_line = out_line + "%s,"%(epi_id)
	if (i+1) == len(cluster_line):
		print out_line[:-1]	
