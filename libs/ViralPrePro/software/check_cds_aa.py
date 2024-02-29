from __future__ import print_function
import sys


f_cds = open(sys.argv[1], 'r').readlines()
f_aa = open(sys.argv[2], 'r').readlines()


def getIds(lines):
	ids = set()
	for l in lines:
		if l.startswith('>'):
			id = l.replace('>','').replace('\n','').replace('cds:','').replace('EPI','')
			ids.add(id)
	return ids

def write_to_file(lines, diff, name):
	nlines = 0
	f = open(name ,'w')
	b = True
	for l in lines:
		nlines+=1
		tmp = l
		if l.startswith('>'):
			id = tmp.replace('>','').replace('\n','').replace('cds:','').replace('EPI','')
			if id in diff:
				b = False
			else:
				f.write(l)
				b = True
		else:
			if b:
				if len(lines) == nlines:
					f.write(l.replace('\n',''))
				else:
					f.write(l)
	
	f.close()


diff = getIds(f_cds).symmetric_difference(getIds(f_aa))

if (diff): #empty set is false
	n = sys.argv[1].rsplit('.',1) 
	name = n[0] + '_purged.' + n[1]
	#print(name)
	write_to_file(f_cds, diff, name)
	n = sys.argv[2].rsplit('.',1) 
	name = n[0] + '_purged.' + n[1]
	#print(name)
	write_to_file(f_aa, diff, name)
	
	print('Both files have not the same amount of sequences with the same identifier!\nThe following sequences were removed:')
	for i in diff:
		print(i)
else:
	print('Both files have the same identifier.')

