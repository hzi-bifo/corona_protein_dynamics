import sys, json
import time_period as tp

period = "month"
lines = sys.stdin.read()
seq_counts = {}
for line in lines.split("\n")[:-1]:
	l = line.split(" ")
	date = tp.get_time_period(l[0], period)
	weight = int(l[1].split("=")[1])
	try:
		seq_counts[date] = seq_counts[date] + weight
	except:
		seq_counts[date] = weight

f = open(sys.argv[1],'w')
sep = " "
numIsolate = ""
for i, k in enumerate(sorted(seq_counts.keys())):#
	numIsolate = numIsolate +str(seq_counts[k] ) + sep
f.write(numIsolate[:-1]+"\n")
