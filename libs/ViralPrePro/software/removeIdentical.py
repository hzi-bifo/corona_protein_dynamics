from sys import argv, exit

def help():
	print
	print "Welcome, User!"
	print 
	print "You can remove all sequences with identical content in a multifasta file"
	print "containing coding sequences (cds) or amino acid sequences (aas)."
	print "Start: removeIdentical.py <cds.fa> <aas.fa> <name_cds_outputfile.fa> <name_aas_outpufile.fa>"
	print "It is possible to exchange cds and aas to remove identical aas sequences"
	print
	exit(0)

def loadFasta(name_file):
	# load fasta file and save in dict
	try:	
		infile = open(name_file, 'r')
		sinseq = infile.read().split(">")
	
		seq = {} # dict, fasta-header, sequence
		
		for i in sinseq:
			cur_seq = i.split('\n',1)
			if len(cur_seq) > 1:
				seq[cur_seq[0]] = cur_seq[1]
		
		return seq
	except:
		print 'Not possible to load '+name_file+'. Check input or dissable sequence collapsing!'

def removeIdentical(cds, aas):
	# removes identical sequences in cds and the entry
	# in the corresponding aas
	try:
		marker = {}
		for i in cds:
			marker[i] = 1
		
		for i in cds:
			if marker[i] == 1:
				for j in cds:
					if marker[j] == 1 and i != j:
						seq1 = cds[i].replace('\n','')
						seq2 = cds[j].replace('\n','')
						if (seq1 == seq2):
							marker[j] = 0
		new_cds = {}
		new_aas = {}
		for i in marker:
			if marker[i] == 1:
				new_cds[i] = cds[i]
				new_aas[i] = aas[i]
		
		if len(new_cds) != len(new_aas):
			print 'Warning, generated aas and cds differ by '+(((len(new_cds)-len(new_aas))**2)**0.5)+' entries !'
		return (new_cds, new_aas)
		
	except:
		print 'Error in removing identical cds. Check input or dissable sequence collapsing!'

def generateOutput(seq, name_seq_out):
	# generate output file
	try:
		outfile = open(name_seq_out, 'w')
		for i in seq:
			outfile.write(">"+i+'\n'+seq[i])	
	except:
		print 'Could not create output files.'
	

if __name__ == "__main__":
	# check input
	if len(argv) != 5:
		help()

	try:
		name_cds_in  = argv[1]
		name_aas_in  = argv[2]
		name_cds_out = argv[3]
		name_aas_out = argv[4]
	except:
		print "Check input and output files"
		exit(1)
	
	# load cds (coding sequences) and aas (amino acid sequences)
	cds =  loadFasta(name_cds_in)
	aas =  loadFasta(name_aas_in)

	# remove identical sequences
	cds, aas = removeIdentical(cds, aas)
	
	# generate output files
	generateOutput(cds, name_cds_out)
	generateOutput(aas, name_aas_out)
