import sys

def adapt(header,name,aa):
	h = header.split("|")
	if aa:
		return str(">" + h[1].replace("_ISL_","")+ '|'+ name + "|" + "|".join(h[0:-1])+"|" + h[0].split("/")[0]  +"|"+ h[-1]).replace("|", " | ")
	else:
		return str(">" + h[1].replace("EPI_ISL_","") + '|'+ name + "|" + "|".join(h[0:-1])+"|" + h[0].split("/")[0]  +"|"+ h[-1]).replace("|", " | ")
		

f = open(sys.argv[1])
lines=f.readlines()
f.close()
name=sys.argv[2]

out_file = open(sys.argv[1].replace("_all",""),"w")
aa=True

if 'cds' in sys.argv[1]:
	aa = False

for  line in lines:
	if line.startswith(">"):
		header = adapt(line[1:],name,aa)
		out_file.write(header)
				
	else:
		out_file.write(line)
out_file.close()
