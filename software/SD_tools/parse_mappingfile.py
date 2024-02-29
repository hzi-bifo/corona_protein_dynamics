import sys, json, re
import time_period as tp

mapping = {}


for line in open(sys.argv[1]).readlines():
	line_split = line.replace("\n","").split("\t")
	mapping_id = line_split[0]
	epi_id = line_split[1].split(" | ")[3]
	mapping[mapping_id] = epi_id

with open(sys.argv[2], 'w') as fp:
    json.dump(mapping, fp)
