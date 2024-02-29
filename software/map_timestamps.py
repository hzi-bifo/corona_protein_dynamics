import sys, re
file_name = sys.argv[1]
f = open(file_name,'r')
lines = f.readlines()
f.close()
out_file = open(file_name.replace("_orig","") ,"w")
#out_file = open("test.fa","w")

protein = sys.argv[2]

time_map ={
"2019-12" : "2000-01-01",
"2020-01" : "2000-06-01",
"2020-02" : "2001-01-01",
"2020-03" : "2001-06-01",
"2020-04" : "2002-01-01",
"2020-05" : "2002-06-01",
"2020-06" : "2003-01-01",
"2020-07" : "2003-06-01", 
"2020-08" : "2004-01-01",
"2020-09" : "2004-06-01",
"2020-10" : "2005-01-01",
"2020-11" : "2005-06-01", 
"2020-12" : "2006-01-01", 
"2021-01" : "2006-06-01",
"2021-02" : "2007-01-01",
"2021-03" : "2007-06-01",
"2021-04" : "2008-01-01"
}

out_map = open(file_name.replace("orig.fa","timing_map.csv") ,"w")
#out_map = open("timing_map.csv" ,"w")

dates = set()


print_out = False
for  line in lines:
	if line.startswith(">"):
		#print line.replace("\n","")
		for match in re.finditer("\d{4}-\d{2}-\d{2}",line):
			date = line[match.start():match.end()]
			month = date.rsplit("-",1)[0]
			dates.add(month)
			try:
				#print date ,'->', time_map[month]
				new_header = line.replace(date,time_map[month])
				print_out = True
			except:
				print "INVALID"
				print_out = False
				
	else:
		if print_out:
			i = "%s%s"%(new_header,line)
			#print i
			j = "%s\t%s\n"%(date,time_map[month])
			out_file.write(i)
			out_map.write(j)
			print_out=False

f_map = open(protein + "_time_map.csv", "w")

for t in sorted(i for i in list(dates) if i in time_map.keys()):
#for t in sorted(list(dates)):
	#f_map.write("%s\t%s\n"%(t,time_map[t]))
	f_map.write("%s\n"%t)


f_map.close()
out_file.close()
out_map.close()
