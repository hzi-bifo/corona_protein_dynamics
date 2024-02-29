import sys, json


f = open(sys.argv[1])
lines = f.readlines()
f.close()

mapping = {}
with open(sys.argv[2], 'r') as fp:
	mapping = json.load(fp)

f = open(sys.argv[1],"w")
for line in lines:
	old = line.replace("\n","")
	new = mapping[old]
	f.write(new+"\n")
f.close()
