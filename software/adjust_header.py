import sys, re
file_name = sys.argv[1]
f = open(file_name,'r')
lines = f.readlines()
f.close()
outfile_name = sys.argv[2]
period = sys.argv[3]
protein_name = sys.argv[4]

#####
#First, get all time periods
time_period_ids = set()
for  line in lines:
	if line.startswith(">"):
		matches = list(re.finditer("\d{4}-\d{2}-\d{2}",line))
		if len(matches) == 1:
			match = matches[0]
			index = 0 if "year" == period else 1 if period == "month" else 2
			key = "-".join(line[match.start():match.end()].split("-")[:(index+1)])
			time_period_ids.add(key)

tp = sorted(list(time_period_ids))

#####
#todo we don't need season for SARS-Cov-2
# Second, map to season periods
# time_map = {}
# season = (2000,1,1)
# for time in tp:
# 	season_string = "%i-0%i-0%i"%(season)
# 	time_map[time] = season_string
# 	season  = (season[0],6,season[2]) if season[1] == 1  else (season[0]+1,1,season[2])

time_map = {}
# season = (2000,1,1)
for time in tp:
    
	# season_string = "%i-0%i-0%i"%(season)
	time_map[time] = time + "-01"
	# season  = (season[0],6,season[2]) if season[1] == 1  else (season[0]+1,1,season[2])


#####
#Third, adjust header and save to files
def adjust_header(header, time_map):
	for match in re.finditer("\d{4}-\d{2}-\d{2}",header):
		index = 0 if "year" == period else 1 if period == "month" else 2
		key = "-".join(header[match.start():match.end()].split("-")[:(index+1)])
		date = header[match.start():match.end()]
		new_date = time_map[key]
		new_header = header.replace(date,new_date)
	return new_header, date, new_date



out_file = open(outfile_name, "w")
file_time_stamp = open(protein_name + "_time_stamp_map.csv","w")
for line in lines:
	if line.startswith(">"):
		header = line.replace("\n","")
		new_header, date, new_date = adjust_header(header, time_map)
		out_file.write("%s\n"%new_header)
		file_time_stamp.write("%s\t%s\n"%(date,new_date))
	else:
		out_file.write(line)

out_file.close()
