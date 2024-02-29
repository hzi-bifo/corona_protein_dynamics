import sys, os

lineage_file = open(sys.argv[1])
lineage_lines = lineage_file.readlines()

mutation_file = open(sys.argv[2])
mutation_list = mutation_file.readlines()

directory = sys.argv[3]

protein = sys.argv[4]

lineages = [l.replace("\n","") for l in lineage_lines]

mutation_list = [m.replace("\n","") for m in mutation_list]
for lineage in lineages:
	l_m = {}
	for mut in mutation_list:
		filename = "%s_lineages_statistics.csv"%mut
		statistics_file = open(directory+"/"+filename)
		b = False
		for line in statistics_file.readlines()[1:]:
			l = line.replace("\n","").split("\t")
			li = l[0]
			fr = l[2]
			if li == lineage:
				#if fr >= 0.8:
				#	l_m[mut] = 1
				#else:
				#	l_m[mut] = 0
				l_m[mut] = fr
				b = True
				break
		if not b:
			l_m[mut] = 0.0
	out_file = open("%s_%s.txt"%(protein,lineage) ,"w")
	out_file.write("%s\t%s\n"%("substitution","frequency"))
	for m in mutation_list:
		out_file.write("%s\t%s\n"%(m, l_m[m]))
	out_file.close()
