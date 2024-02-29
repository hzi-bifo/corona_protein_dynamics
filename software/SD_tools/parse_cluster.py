import sys, json, re

cluster = {}
for line in open(sys.argv[1]).readlines()[1:]:
	line_split = line.replace("\n","").split("\t")
	#rep = int(re.sub('[^0-9]' ,'',line_split[0]))
	rep = line_split[0]
	if len(line_split) > 1:
		for epi_id in line_split[1].split(","):
			try:
				cluster[rep] = cluster[rep] + [epi_id]
			except:
				cluster[rep] = [epi_id]
		
with open(sys.argv[2], 'w') as fp:
    json.dump(cluster, fp)
