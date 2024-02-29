from Bio import AlignIO, SeqIO
import sys, re, distance, os,random, datetime

def get_season(d):
	d = d.split('-')
	year = int(d[0])
	month = int(d[1])
	if month in range(4,10):
		rtn = "%iS"%year
	elif month in range(10,13):
		rtn = "%iN"%(year+1)
	else:
		rtn = "%iN"%(year)
	return rtn

def get_week(d):
	year, month, day = int(d.split("-")[0]), int(d.split("-")[1]), int(d.split("-")[2])
	week = datetime.date(year, month, day).strftime("%V")
	if week == "01" and month == 12:
		year +=1
	elif week == "53" and month == 1:
		year -= 1
	return "%i_week%s" %(year, week)

file_name = sys.argv[1]
records = list(SeqIO.parse(file_name, "fasta"))
#root = sys.argv[2]

#sample_size = int(sys.argv[3])
period = sys.argv[2]

if not (period in ["day", "week", "month", "year", "season"]):
	print "Aborted! Time period is incorrect. Please choose between: day, month or year"
	quit()

time_period_couunt = {}
time_period_ids = {}

root_time_period=""
root_id=""
header_seq_map = {}
id_header_map = {}

day_season_map = {}
season_day_map = {}
season_count_map = {}

count_na = 0
for nuc in records:
	#filter for correct time stamp
	#count sequences per time perios
	matches = list(re.finditer("\d{4}-\d{2}-\d{2}",nuc.id))
	if len(matches) == 1:
		match = matches[0]
		index = 0 if "year" == period else 1 if period == "month" else 2
		key = "-".join(nuc.id[match.start():match.end()].split("-")[:(index+1)])
		id_split = nuc.id.split("|")
		ID = id_split[1]  #get the EPI_ISL ID
		header_seq_map[nuc.id] = nuc.seq
		id_header_map[ID] = nuc.id
#		if root in nuc.id:
#			root_id = ID
#			if period == "season":
#				root_time_period = get_season(key)
#			else:
#				root_time_period = key
		if period == "season" or period == "week":
			if period == "season":
				s = get_season(key)
			else:
				s =  get_week(key)
				print key, s
			day_season_map[key] = s
			try:
				season_day_map[s] = season_day_map[s] + [key]
				season_count_map[s] = season_count_map[s] + 1
			except:
				season_day_map[s] = [key]
				season_count_map[s] = 1
			key = s
		try:
			time_period_ids[key] = time_period_ids[key] + [ID]
		except:
			time_period_ids[key] = [ID]
	else:
		count_na += 1#count sequences with incorrect time stamps
#if root_id == "":
#	print "Root not found", root
#	quit()
#####
#get missing dates
from datetime import datetime, timedelta
from collections import OrderedDict
if not period == "week":
	tp_keys = time_period_ids.keys()
else:
	tp_keys = day_season_map.keys()
s = sorted(tp_keys)[0]
e = sorted(tp_keys)[len(tp_keys)-1]
if period == "day" or period == "week":
	s_y, s_m, s_d = int(s.split("-")[0]), int(s.split("-")[1]), int(s.split("-")[2])
	e_y, e_m, e_d = int(e.split("-")[0]), int(e.split("-")[1]), int(e.split("-")[2])
	start = datetime(year=s_y, month=s_m, day=s_d)
	end = datetime(year=e_y, month=e_m, day=e_d)
	months = OrderedDict(((start + timedelta(_)).strftime("%Y-%m-%d"), 0) for _ in range((end - start).days)).keys() +[e]
	if period == "week":
		import datetime	#somehow this reduces an error
		months = [get_week(m) for m in months]
		tp_keys = time_period_ids.keys()
elif period == "month":
	s_y, s_m, s_d = int(s.split("-")[0]), int(s.split("-")[1]), 1
	e_y, e_m, e_d = int(e.split("-")[0]), int(e.split("-")[1]), 1
	start = datetime(year=s_y, month=s_m, day=s_d)
	end = datetime(year=e_y, month=e_m, day=e_d)
	months = ["-".join(a.split("-")[:index+1]) for a in OrderedDict(((start + timedelta(_)).strftime("%Y-%m-1"), 0) for _ in range((end - start).days)).keys()] + [e]
elif period == "year":
	s_y, s_m, s_d = int(s.split("-")[0]), 1, 1
	e_y, e_m, e_d = int(e.split("-")[0]), 1, 1
	start = datetime(year=s_y, month=s_m, day=s_d)
	end = datetime(year=e_y, month=e_m, day=e_d)
	months = [ "-".join(a.split("-")[:index+1]) for a in  OrderedDict(((start + timedelta(_)).strftime("%Y-1-1"), 0) for _ in range((end - start).days)).keys()] + [e]
elif period == "season":
	months = time_period_ids.keys()

#####
#print sequence statistic
f_out = open("seq_counts_%s.csv"%sys.argv[1].replace('.fasta',''),"w")
f_out.write("date\tnum\n")
f_out.write("NA\t%i\n"%count_na)
for m in sorted(list(set(months))):
	if m in tp_keys:
		f_out.write("%s\t%i\n"%(m, len(time_period_ids[m])))
	else:
		f_out.write("%s\t%i\n"%(m, 0))
f_out.close()

#####
#sample sequences per month
#if sample_size > 0:	#else no sampling
#	for time_period in sorted(time_period_ids.keys()):
#		ids = time_period_ids[time_period]
#		max_size = sample_size
#		if len(ids) > sample_size:	# subsample
#			if root_time_period == time_period:
#				max_size = sample_size-1#-1 because we add the root at the end
#			time_period_ids[time_period] = [ids[i] for i in random.sample(range(len(ids)-1) , max_size)]
#					
#		else:	
#			if True:	#no up-sampling
#				time_period_ids[time_period] = ids
#			else:	# sample with replacement
#				if root_time_period == time_period:
#					max_size = sample_size-1#-1 because we add the root at the end
#				time_period_ids[time_period] = [ids[i] for i in [random.randint(0, len(ids)-1) for p in range(0, max_size)]]
			
#####
#detect and find all filtered sequences
#for time_period in sorted(tp_keys):
#	file_out = open("sequences_%s.fa"%time_period,"w")
#	if root_time_period == time_period:
#		header = id_header_map[root_id]				
#		file_out.write(">%s\n"%header)
#		file_out.write("%s\n"%header_seq_map[header].upper())
#		#file_out.write("%s\n"%header_seq_map[header][(1*3):(520*3)+3].upper())
#	for ID in time_period_ids[time_period]:	
#		header = id_header_map[ID]				
#		file_out.write(">%s\n"%header)
#		file_out.write("%s\n"%header_seq_map[header].upper())
#		#file_out.write("%s\n"%header_seq_map[header][(1*3):(520*3)+3].upper())
#	file_out.close()
