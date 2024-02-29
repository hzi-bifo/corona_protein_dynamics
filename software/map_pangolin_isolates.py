import sys


meta_file = open(sys.argv[1])
meta_line = meta_file.readlines()

aliases_file = open(sys.argv[2])
aliases_line = aliases_file.readlines()

aliases_dict = {}
for line in aliases_line:
	li = line.replace(" ","").replace("\n","").split("\t")
	aliases_dict[li[0]]= li[1]

pango_dict ={}
for line in meta_line[1:]:
	li = line.split("\t")
	try:
		pango_dict[li[18]] = pango_dict[li[18]] + [li[2]]
	except:
		pango_dict[li[18]] = [li[2]]

pango_dict2 ={}
for k1 in sorted(pango_dict.keys()):
	pango_dict2[k1] = pango_dict[k1]
	for k2 in sorted(pango_dict.keys()):
		if k2 in aliases_dict.keys():
			tmp = aliases_dict[k2]
			if "%s."%k1 in tmp:
                                pango_dict2[k1] = pango_dict2[k1] + pango_dict[k2]
		elif "%s."%k1 in k2:
				pango_dict2[k1] = pango_dict2[k1] + pango_dict[k2]

pango_file = open("all_lineages.csv","w")
for k in pango_dict2.keys():
	f = open("%s_isolates.csv"%k ,"w")
	pango_file.write("%s\n"%k)
	for l in pango_dict2[k]:
		f.write("%s\n"%l)
	f.close()
pango_file.close()
