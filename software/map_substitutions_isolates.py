import sys, re, os

mutation_file = open(sys.argv[1])
mutation_lines = mutation_file.readlines()

#sign_res_file = open(sys.argv[2])
#sign_res_lines = sign_res_file.readlines()

mapping_file = open(sys.argv[2])
mapping_lines = mapping_file.readlines()

cluster_file = open(sys.argv[3])
cluster_lines = cluster_file.readlines()

allelePath = sys.argv[4]

prev=-1
count=0
mutation_list = open("mutation_list.csv","w")

map_dict = {}
cluster_dict = {}

for mapping_line in mapping_lines:
	map_splits = mapping_line.replace("\n","").split("\t")
	map_dict[map_splits[0]] = map_splits[1]

for cluster_line in cluster_lines:
	cluster_split = cluster_line.replace("\n","").split("\t")
	rtn = []
	if len(cluster_split) > 1:
		rtn = [ cl for cl in cluster_split[1].split(",")]
	cluster_dict[cluster_split[0]] = rtn

for line in range(len(mutation_lines)):
	m=mutation_lines[line].split("\t")
	mutation=m[0]
	size=int(m[2].replace("\n",""))
	pos = int(re.sub('[^0-9]','',mutation))
	#print "mut", mutation
	if pos == prev:
		count +=1
	else:
		count = 0
	if m[1] == "1":
		sign="_sign"
	else:
		sign=""
	alleles_file = open(allelePath+"%i.alleleAssociation.txt"%pos)
	alleles_lines = alleles_file.readlines()
	strng_out = "%s_%i%s"%(mutation, count, sign)
	isolates_out = open("%s_isolates.csv"%strng_out,'w')
	mutation_list.write(strng_out+"\n")
	opl1 = 0
	for allele in alleles_lines:
		a = allele.split("\t")
		id_ = a[0].replace("\n","")
		count_n =a[2]
		if not count_n == '':
			s = count_n.split(",")
			for p in range(len(s)):
				if int(s[p]) == count and p+1 == len(s):#checks if another substitution later in time occurs
					iso_header = map_dict[id_]
					iso_out = iso_header.split(" | ")
					weight=int(iso_out[6].split("=")[1])
					epi_id=iso_out[3]
					isolates_out.write(epi_id+"\n")
					opl1+=1
					for cl in cluster_dict.keys():
						if epi_id == cl:
							for rtn in cluster_dict[cl]:#c[1].split(","):
								opl1+=1
								isolates_out.write(rtn.replace("\n","")+"\n")
	alleles_file.close()
	isolates_out.close()
	prev=pos
