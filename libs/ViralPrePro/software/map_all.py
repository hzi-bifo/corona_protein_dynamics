import sys


f = open(sys.argv[1]).readlines()
f2 = open(sys.argv[2]).readlines()
for li in f2:
	o = li.replace('\n','')
	if o.startswith('>'):
		for line in f:
			l = line.replace('\n','').split('\t')
			#print l[0]
			#print l[1]
			if ':' in l[0]:
				header = l[0].split(':')[1] 
			else:
				header = l[0]
			if header.split(' ')[0] in o:
				#print header.split(' ')[0] , 'in' , o
				print '>%s' %l[1]
				break
	else:
		print o

