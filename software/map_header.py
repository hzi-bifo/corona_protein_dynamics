import sys

def adapt(header):
	h = header.split("|")
	return str(">" + h[1]+ '|'+ "complete genome" + "|" + "|".join(h[0:-1])+"|" + h[0].split("/")[0]  +"|"+ h[-1]).replace("|", " | ")

f = open(sys.argv[1])
lines=f.readlines()
f.close()

out_file = open("new_"+sys.argv[1],"w")

for  line in lines:
	if line.startswith(">"):
		header = adapt(line[1:])
		out_file.write(header)
				
	else:
		out_file.write(line)
out_file.close()
