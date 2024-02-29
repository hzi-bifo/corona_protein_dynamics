import sys


f =open(sys.argv[1]).readlines()
count = 1

for line in f:
	#id = line.split(' ')[1]
	#print id
	#print line.split('|')[1]
	
	print line.replace('>','').replace('\n','') + '\t' + 'fdp0%i'%count
	count+=1
