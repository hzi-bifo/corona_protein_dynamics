from sys import argv
def help():
	print "\nProgram call: python2 removeSignalPeptide.py <PathToMultiFasta.fa> <cds_or_aas> <PeptideLength | Subtype>"
	print "Example     : python2 removeSignalPeptide.py cds_orig.fa cds 17"
	print "Example     : python2 removeSignalPeptide.py aa_orig.fa aas H3N2"
	print "Following Subtypes are included:\nH3N2\nH2N2\nH1N1\npH1N1\n"

def loadFasta(fasta):
        # load Fasta File and Store in dict
        out      = {}
        infile   = open(fasta, 'r')
        seqences = infile.read().split('>')

        for seq in seqences:
                if len(seq) > 1:
                        tmp = seq.split('\n', 1)
                        out[('>'+tmp[0])] = tmp[1]
        return out

def printFasta(fasta, scheme):
	for seq in fasta:
        	print seq
                print fasta[seq][scheme:],

def makeNumbering(fasta, cds_aas, scheme):
	# remove the signal peptide
	try:
		scheme = int(scheme)
		printFasta(fasta, scheme)
	except:
		if scheme == 'H3N2' and cds_aas == 'aas':
			printFasta(fasta, 16)
		if scheme == 'H3N2' and cds_aas == 'cds':
                        printFasta(fasta, 48)
		if scheme == 'H2N2' and cds_aas == 'aas':
                        printFasta(fasta, 15)
                if scheme == 'H2N2' and cds_aas == 'cds':
                        printFasta(fasta, 45)
		if scheme == 'H1N1' and cds_aas == 'aas':
                        printFasta(fasta, 18)
                if scheme == 'H1N1' and cds_aas == 'cds':
                        printFasta(fasta, 54)
		if scheme == 'pH1N1' and cds_aas == 'aas':
                        printFasta(fasta, 17)
                if scheme == 'pH1N1' and cds_aas == 'cds':
                        printFasta(fasta, 34)
		
if __name__ == "__main__":
	# remove the signal peptide of all sequences in a multi fasta file
	try:
		makeNumbering(loadFasta(argv[1]), argv[2], argv[3])
	except:
		help()
