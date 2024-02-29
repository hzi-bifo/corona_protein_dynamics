import sys, json, re ,os

metadata = {}
with open(sys.argv[1], 'r') as fp:
    metadata = json.load(fp)

mapping = {}
with open(sys.argv[6], 'r') as fp:
    mapping = json.load(fp)

mut_iso = {}
mut_names = []
def parse_files(files):
	#for i, l in enumerate(sorted( files,  key = lambda x: int(x.split("_")[1])  )):
		#mut = l.split("_isolates.")[0]
	for i, l in enumerate(sorted( files,  key = lambda x: int(x.split("_")[1].split(".")[0])  )):
		lines = open(l).readlines()
		#if 2 < len(lines):
		mut = l.split(".txt")[0]
		mut_names.append(mut)
		for line in lines:
			iso = int(re.sub('[^0-9]' ,'',line.replace("\n","")))
			#iso = int(re.sub('[^0-9]' ,'',mapping[line.replace("\n","")]))
			try:
				mut_iso[mut].add(iso)
			except:
				mut_iso[mut] = set([iso])
#	print mut_iso	

files = sys.stdin.read().replace("\n","").split(" ")
parse_files(files)
keys = sorted(mut_iso, key=lambda k: len(mut_iso[k]), reverse=True)
mut_iso_corr = {}

for k1 in mut_iso.keys():
	tmp = set()
	for k2 in keys:
		if (not k1 is k2) and len(mut_iso[k1]) > len(mut_iso[k2]):
			tmp |= mut_iso[k2]
	x = (mut_iso[k1] - tmp)
	mut_iso_corr[k1] = x

times =  [ line.replace("\n","") for line in open(sys.argv[2]).readlines()]
#time_num = [ float(l) for line in open(sys.argv[3]).readlines() for l in line.split("\t")]

filename = sys.argv[4]
if os.path.exists(filename):
	aw_ = 'a' # append if already exists
else:
	aw_ = 'w' # make a new file if not

subtree_mut_per_pos_file = open(filename, aw_)


filename = sys.argv[5]
#print filename, os.path.exists(filename)
if os.path.exists(filename):
	aw = 'a' # append if already exists
else:
	aw = 'w' # make a new file if not
mut_per_pos_file = open(filename, aw)
#print mut_names
first = True
#for k in mut_names:	#mutation
for k in mut_iso.keys():
	mut = k.split("_")[0]
	#print mut
	#print aw
	if aw == 'w': 
		if first:
			mut_per_pos_file.write(mut)
			first = False
		else:
			mut_per_pos_file.write("\t"+mut)
	else:
		mut_per_pos_file.write("\t"+mut)
	nums = [ 0 for _ in range(len(times))]
	for epi_id in mut_iso_corr[k]:	#isolate carrying this mutation
		for i, time in enumerate(times):	#at each time point
			if time in metadata.keys(): 
				if epi_id in metadata[time]:	#check if isolate from time period
					nums[i] += 1 
	subtree_mut_per_pos_file.write("\t".join(map(str,nums)) + "\n")
#	print [float(a)/b  for a, b in zip(nums,time_num)]

#print "mut", "number", "corr_number"
#for k in sorted(mut_iso_corr, key=lambda k: len(mut_iso_corr[k]), reverse=True):
#	if len(mut_iso[k]) != len(mut_iso_corr[k]):
#		print k, len(mut_iso[k]), len(mut_iso_corr[k])
#for t in times:
#	if t in metadata.keys(): 
#		print t, len(metadata[t])
