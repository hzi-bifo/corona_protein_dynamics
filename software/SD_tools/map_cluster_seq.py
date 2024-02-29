import sys, json

f = open(sys.argv[1])
lines = f.readlines()
f.close()

cluster = {}
with open(sys.argv[2], 'r') as fp:
	cluster = json.load(fp)
cluster_keys = cluster.keys()

#f = open(sys.argv[1].replace(".txt","_new.txt"),'w')
f = open(sys.argv[1],'w')
for line in lines:
	epi_id = line.replace("\n","")
	f.write(epi_id + "\n")
	if epi_id in cluster_keys:
		for epi_id2 in cluster[epi_id]:
			#f.write(epi_id2+"_new"+"\n")
			f.write(epi_id2+"\n")
f.close()
