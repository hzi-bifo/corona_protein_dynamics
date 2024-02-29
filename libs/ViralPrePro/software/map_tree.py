import sys


f =open(sys.argv[1]).readlines()
f2 = open(sys.argv[2]).readlines()
for li in f2:
	o = li.replace('\n','')
	if '[&U]' in o:
		for line in f:
			l = line.replace('\n','').split('\t')
			#print l[0]
			#print l[1]
			#print l[0] in o
			s = l[0].split('-')[0]
			if s in o:
				o = o.replace(s,l[1])
			else:
				print l[0]
				print l[1]
				print l[0] in o
		print o
	else:
		print o

