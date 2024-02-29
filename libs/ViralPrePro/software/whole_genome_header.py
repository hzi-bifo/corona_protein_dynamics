import sys


map_file = open(sys.argv[1])
map_lines = map_file.readlines()

id_dict = {}
for line in map_lines:
	s = line.replace("\n","").split("\t")
	epi_id = s[1].split(" | ")[3]
	id_dict[epi_id] = s[0]

for i in sorted(id_dict.keys()):
	print i, id_dict[i]
genome_file = open(sys.argv[2])
genome_lines = genome_file.readlines()

print
for genome in genome_lines:
	genome = genome.replace("\n","")
	if genome.startswith(">"):
		epi_id = genome.split("|")[1]
		try:	#bas solution, but in case of clustered sequences, this will fail and omit non-clustered sequences
			print ">%s"%id_dict[epi_id]
			prnt_seq = True
		except:
			prnt_seq = False
			
	elif prnt_seq:
		print genome
