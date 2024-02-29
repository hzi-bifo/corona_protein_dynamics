import sys

fasta_file = open(sys.argv[1])
fasta_lines = fasta_file.readlines()

err_msg = "The input file is not correct. Please revise the file %s and comare with example from Readme."%sys.argv[1]
for line in fasta_lines:
	if line.startswith(">"):
		if line.count("|") == 2:
			li = line.split("|")
			name = li[0]
			id_ = li[1]
			date = li[2]
			if not name.count("/") == 3:
				print err_msg
				quit()
		else:
			print err_msg
			quit()
