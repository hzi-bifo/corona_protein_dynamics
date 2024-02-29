import sys

table_file = open(sys.argv[1])
table_lines = table_file.readlines()

lin_size_file = open(sys.argv[2])
lin_size_lines = lin_size_file.readlines()

subs_size_file = open(sys.argv[3])
subs_size_lines = subs_size_file.readlines()

out_name = sys.argv[4]


lin_size = {}
for line in lin_size_lines:
	s = line.replace("\n","").split("\t")
	lin_size[s[0]] = float(s[1])

subs_size = {}
for line in subs_size_lines:
	s = line.replace("\n","").split("\t")
	subs_size[s[0]] = float(s[1])

head_line = table_lines[0].replace("\n","")
heatmap_file = open(out_name,'w')
heatmap_file.write("%s\n"%head_line)

for line_nr in range(1, len(table_lines)):
        li=table_lines[line_nr].replace("\n","").split("\t")
        subs = li[0]
	su_size = subs_size[subs]
        overlaps = [float(i) for i in li[1:]]
	over_subs_ratio = [ o/su_size for o in overlaps]
	i = 0
	over_lin_ratio = []
	for k in range(len(overlaps)):
		li_size = lin_size[ head_line.split("\t")[k+1]  ]
		over_lin_ratio.append(overlaps[i]/li_size)
		i+=1
	means = [ (over_subs_ratio[i] + over_lin_ratio[i])/ 2. for i in range(len(overlaps)) ]
	max_mean = max(means)
	rtn = []
	for i in range(len(overlaps)):
		#if overlaps[i] > 0 and means[i] == max_mean:#(over_subs_ratio[i] > 0.5 or over_lin_ratio[i] > 0.5) :
		#	print "%s\t%s\t\t%i\t%i\t%i\t%.2f\t%.2f\t%.5f" %(subs, head_line.split("\t")[i+1], overlaps[i], subs_size[subs], lin_size[head_line.split("\t")[i+1] ], over_subs_ratio[i]*100, over_lin_ratio[i]*100, means[i] * 100 )
		if means[i] == max_mean and overlaps[i] > 1:
			rtn.append(over_lin_ratio[i])
		else:
			rtn.append(0)
	heatmap_file.write("%s"%subs)
	for r in rtn:
		heatmap_file.write("\t%f"%r)
	heatmap_file.write("\n")
