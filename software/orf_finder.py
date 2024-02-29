import re, sys, distance
from Bio import SeqIO
from string import maketrans

def translate(seq):
	table = {
		'ATA':'I', 'ATC':'I', 'ATT':'I', 'ATG':'M',
		'ACA':'T', 'ACC':'T', 'ACG':'T', 'ACT':'T',
		'AAC':'N', 'AAT':'N', 'AAA':'K', 'AAG':'K',
		'AGC':'S', 'AGT':'S', 'AGA':'R', 'AGG':'R',                 
		'CTA':'L', 'CTC':'L', 'CTG':'L', 'CTT':'L',
		'CCA':'P', 'CCC':'P', 'CCG':'P', 'CCT':'P',
		'CAC':'H', 'CAT':'H', 'CAA':'Q', 'CAG':'Q',
		'CGA':'R', 'CGC':'R', 'CGG':'R', 'CGT':'R',
		'GTA':'V', 'GTC':'V', 'GTG':'V', 'GTT':'V',
		'GCA':'A', 'GCC':'A', 'GCG':'A', 'GCT':'A',
		'GAC':'D', 'GAT':'D', 'GAA':'E', 'GAG':'E',
		'GGA':'G', 'GGC':'G', 'GGG':'G', 'GGT':'G',
		'TCA':'S', 'TCC':'S', 'TCG':'S', 'TCT':'S',
		'TTC':'F', 'TTT':'F', 'TTA':'L', 'TTG':'L',
		'TAC':'Y', 'TAT':'Y', 'TAA':'_', 'TAG':'_',
		'TGC':'C', 'TGT':'C', 'TGA':'_', 'TGG':'W',
	}
	protein =""
	if len(seq)%3 == 0:
		for i in range(0, len(seq), 3):
			codon = seq[i:i + 3]
			try:
				protein+= table[codon]
			except:
				protein += "X"
	return protein	

records = SeqIO.parse(sys.argv[1], "fasta")
out_file = open(sys.argv[2],"w")
pattern = re.compile(r'(?=(ATG(?:...)*?)(?=TAG|TGA|TAA))')

protein = 1272 #size of spike protein
start = "MFVF"
threshold = 4

def orfs(dna, t):
	return [i for i in list(set(pattern.findall(dna))) if len(i) in  range((protein-t)*3,(protein+t)*3) ]

def distance_list(gen, ref):
	return distance.nlevenshtein(gen, ref)

count = 0
seq = 0
none = 0
for record in records:
	seq += 1.
	orf = orfs(str(record.seq), threshold)
	if len(orf) >= 1:
		orf.sort(key=lambda x:distance_list(translate(x[:12]),start))
		for gen in orf:
			if distance.nlevenshtein(translate(gen[:12]) , start) <= 0.25:	#check if start region is the same
				out_file.write(">%s\n"%record.id)
				out_file.write("%s\n"%gen)
				count+=1.
				break
		else:
			none += 1.
	else:
		none += 1.
out_file.close()


stat_file = open("orf_statistics.csv",'w')
stat_file.write("Sequences\t%i\n"%seq)
stat_file.write("Count\t%i\n"%count)
stat_file.write("Ratio\t%f\n"%(count/seq))
stat_file.write("Nothing\t%i\n"%none)
stat_file.close()
