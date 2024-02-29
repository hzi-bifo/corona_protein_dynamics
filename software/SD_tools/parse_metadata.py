import sys, json, re
import time_period as tp

metadata = {}
period = "month"


for line in open(sys.argv[1]).readlines()[1:]:
	line_split = line.replace("\n","").split("\t")
	epi_id = int(re.sub('[^0-9]' ,'',line_split[2]))
	date = tp.get_time_period(line_split[4], period)
	try:
		metadata[date] = metadata[date] + [epi_id]
	except:
		metadata[date] = [epi_id]


with open(sys.argv[2], 'w') as fp:
    json.dump(metadata, fp)
