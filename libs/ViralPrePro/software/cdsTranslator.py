#from Bio.Seq import Seq
#from Bio.Alphabet import generic_dna
from sys import argv, exit

header = []	# contains all header files
blocks = []	# contains all sequences

def translate(cd):  # returns the amino acid of a codon unsing the standard table
	# Standard coding table
	AAs   = "FFLLSSSSYY**CC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG"
	Base1 = "TTTTTTTTTTTTTTTTCCCCCCCCCCCCCCCCAAAAAAAAAAAAAAAAGGGGGGGGGGGGGGGG"
	Base2 = "TTTTCCCCAAAAGGGGTTTTCCCCAAAAGGGGTTTTCCCCAAAAGGGGTTTTCCCCAAAAGGGG"
	Base3 = "TCAGTCAGTCAGTCAGTCAGTCAGTCAGTCAGTCAGTCAGTCAGTCAGTCAGTCAGTCAGTCAG"
	
	if len(cd) != 3:
		return "X"
	
	c = 0	# Position Counter
	# determine correct amino acid for cd (codon)
	for B1 in Base1:
		if cd[0] == B1:
			for B2 in Base2[c:c+16]:
				if cd[1] == B2:
					for B3 in Base3[c:c+4]:
						if cd[2] == B3:
							return AAs[c]
						else:
							c+=1
				else:
					c+=1
		else:
			 c+=1
	return "X" # return X for unknown codons

def help():
	print 
	print "########## Welcome to cdsTranslator ##############################"
	print 
	print "Input: cds or nucleotide sequence als fasta or multifasta file .fa"
	print "Call: python2 cdsTranslator <Multifastafile.fa>"
	print "For unknown characters, cdsTranslator return: X"
	print	

if len(argv) != 2:
	print help()
	exit()

infile=open(argv[1], 'r')
seqens = infile.read().split('>')

# load multifasta file
for i in seqens:	# load all sequences 
	if len(i) >1:
		lines = i.split('\n')
		header.append(lines[0])
		s = ''
		for j in range(1,len(lines)):
			s += lines[j]
		blocks.append(s)
	
# calculate aa for every cd
for i in range(len(blocks)):	# generate aas from cds
	cds = str(blocks[i])
		
	aas = ""		# amino acid sequence
	for j in range(0,len(cds),3):
		aas += translate(cds[j:j+3])	# translate codon

	print ">"+header[i][0:]			# print header
	no_lines = (len(aas)-(len(aas)%70))/70	# format output, 70 chars per line
	
	# print correct output
	if len(aas) > 70:
		start = 0
		end   = 69
		for i in range(no_lines):
			print aas[start:end+1]
			start += 70
			end   += 70
		end   +=((len(aas)%70)-70)
		print aas[start:end]		
	else:
		print aas
