import sys

cluster_file = open(sys.argv[1])
cluster_lines = cluster_file.readlines()

seq_file = open(sys.argv[2])
seq_lines = seq_file.readlines()


id_map = {}
start = True
for i in range(len(cluster_lines)):
	line = cluster_lines[i]
	if line.startswith(">"):
		if not start:
			id_map[id_] = count_seq
		start= False
		count_seq = 0
		seq = ""
	else:
		count_seq+=1
		li = line.split("\t")
		if int(li[0]) == 0:
			id_ = li[1].split(",")[1].replace(" ","").split("...")[0].split("|")[1]
	if i+1 == len(cluster_lines):
		id_map[id_] = count_seq

for line in seq_lines:
	if line.startswith(">"):
		id_ = line.split("|")[1]
		w = id_map[id_]
		print "%s|weight=%i"%(line.replace("\n",""), w)
	else:
		print line.replace("\n","")
