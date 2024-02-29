from Bio import AlignIO, SeqIO
import sys, country_continent, chinese_provinces, random, re, getopt

lc_map = country_continent.lc_map
chinese_provinces_list = chinese_provinces.chinese_provinces_list

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

def is_location(location):
	return location in lc_map.keys()

def filter_sequence_ids(records, geographical="", temporal="" ):
	if geographical == "" and temporal == "":
		return {}
	
	root_id = ""
	root_assignment = ""
	rtn_dict = {}
	id_header_map = {}
	header_seq_map = {}

	for record in records:
		add = True
		epi_id = record.description.split("|")[1]
		if geographical != "":
			location = record.description.split("/")[1]
			country = location
			if not (is_location(location.upper())):
				if location in chinese_provinces_list:
					#continent = 'ASIA'
					location ='China'
				else:
					add = False
			if add and geographical == "country":
				location = location.upper().replace(" ","_")
			if add and geographical == "continent":
				location = lc_map[location.upper()].replace(" ","_")
		if temporal != "":
			matches = list(re.finditer("\d{4}-\d{2}-\d{2}",record.description))
			if len(matches) == 1:
				match = matches[0]
				index = 0 if "year" == temporal else 1 if temporal == "month" else 2
				date = "-".join( record.description[match.start():match.end()].split("-")[:(index+1)])
				if temporal == "season":
					date = get_season(date)
				elif temporal == "week":
					date = get_week(date)
				add &= True
			else:
				add &= False
		if add :
			if geographical != "" and temporal != "":
				assignment = location + "_" + date
			elif geographical != "":
				assignment = location
			elif temporal != "":
				assignment = date
			try:
				id_header_map[epi_id] = record.description
				header_seq_map[record.description] = record.seq
				rtn_dict[assignment] = rtn_dict[assignment] + [epi_id]
			except:
				rtn_dict[assignment] = [epi_id]
			if root in record.id:
				root_id = epi_id
				root_assignment = assignment
	return rtn_dict, id_header_map, header_seq_map, root_id, root_assignment


def sampling(dict_, root_assignment):
	rtn = {}
	for key in dict_.keys():
		ids = dict_[key]
		max_size = sample_size
		if len(ids) > sample_size:
			if root_assignment == key:
				max_size = sample_size -1
			rtn[key] = [ids[i] for i in random.sample(range(len(ids)-1) , max_size)]
		else:	
			if True:	#no up-sampling
				rtn[key] = ids
			else:	# sample with replacement
				if root_assignment == key:
					max_size = sample_size-1#-1 because we add the root at the end
				rtn[key] = [ids[i] for i in [random.randint(0, len(ids)-1) for p in range(0, max_size)]]
	return rtn

def save(rtn_dict, id_header_map, header_seq_map, root_assignment, root_id):
	for key in rtn_dict.keys():
		file_out = open("sequences_%s.fa"%key.replace(' ',''),"w")
		if root_assignment == key:
			header = id_header_map[root_id]				
			file_out.write(">%s\n"%header)
			file_out.write("%s\n"%header_seq_map[header].upper())
			#file_out.write("%s\n"%header_seq_map[header][(490*3):(510*3)+3].upper())
		for epi_id in rtn_dict[key]:	
			header = id_header_map[epi_id]				
			file_out.write(">%s\n"%header)
			file_out.write("%s\n"%header_seq_map[header].upper())
			#file_out.write("%s\n"%header_seq_map[header][(490*3):(510*3)+3].upper())
		file_out.close()


if __name__ == "__main__":
	argv = sys.argv[1:]
	try:
		opts, args = getopt.getopt(argv, "f:s:r:g:t:h")
	except:
		print("Error")
	root, geo, time = "", "", ""
	for opt, arg in opts:
		if opt in ["-f"]:
			file_name = arg
		if opt in ["-s"]:
			try:
				sample_size = int(arg)
			except:
				print arg, "not an integer"
				quit()
		if opt in ["-r"]:
			root = arg
			print root
		if opt in ["-g"]:
			if arg == "continent" or arg == "country":
				geo = arg
			else:
				print arg , "not a correct location"
				quit()
		if opt in ["-t"]:
			if arg in ["day", "week", "month", "year", "season"]:
				time = arg
			else:
				print arg , "not a correct time"
				quit()
		if opt in ["-h"]:
			print "example call: python sampling.py -f gisaid_hcov-19_2021_04_05_10.fasta -s 10 -r hCoV-19/Wuhan/IPBCAMS-WH-01/2019 -g continent -t month"
			quit()
	
	
	records = list(SeqIO.parse(file_name, "fasta"))
	rtn_dict , id_header_map, header_seq_map, root_id, root_assignment = filter_sequence_ids(records, geographical=geo, temporal=time) 
	if sample_size > 0:
		rtn_dict = sampling(rtn_dict, root_assignment)
	#for c in sorted(rtn_dict, key=lambda c: len(rtn_dict[c]), reverse=True):
	#	print "%s\t%i"%(c, len(rtn_dict[c]) )
	print "root id and assignment", root_id, root_assignment
	save(rtn_dict, id_header_map, header_seq_map, root_assignment, root_id)
