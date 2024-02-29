import sys, json


metadata = {}
with open(sys.argv[1], 'r') as fp:
	metadata = json.load(fp)

f = open(sys.argv[2],'w')
sep = " "
numIsolate = ""
for i, k in enumerate(sorted(metadata.keys())[1:]):#omit first entry because NA
	if len( metadata[k] ) > 1000:
		numIsolate = numIsolate + "1000" + sep
	else:
		numIsolate = numIsolate +str(len( metadata[k] )) + sep
f.write(numIsolate[:-1]+"\n")
