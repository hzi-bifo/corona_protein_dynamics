#!/bin/perl
use strict;
use warnings;

my $filename = $ARGV[0];

# set filenames
my $aafilename = $filename . "_aa.fa";
my $dnafilename = $filename . "_cds.fa";

# open fasta file with amino acid sequences
open (my $aafile, $aafilename) or die "Could not open file '$aafilename' $!";

#### parse amino acid file into hash
my $counter = 0;
my $sequence = "";
my $id = "";
my %aaseq = ();
while( my $line = <$aafile>)  {
        if ($line =~ /^>/) {
		$id = "f0dp" . $counter; 	#set new id that will replace fasta header
                $counter = $counter +1;
                $line =~ s/\n//g; 		#remove newline

                $aaseq{ $counter } = {
                        line => $line,
			newid => $id,
                };

                $sequence = "";
        } else {
                $line =~ s/\n//g; 		#remove newline
                $sequence = $sequence . $line;
                $aaseq{ $counter }{sequence} = $sequence;
        }
}

close $aafile;

my $aacounter = $counter;

# open file with nucleotide sequences
open (my $dnafile, $dnafilename) or die "Could not open file '$dnafilename' $!";

#### parse nucleotide file into hash
$counter = 0;
$sequence = "";
$id = "";
my %dnaseq = ();
while( my $line = <$dnafile>)  {
        if ($line =~ /^>/) {
                $id = "f0dp" . $counter;         #set new id that will replace fasta header
                $counter = $counter +1;
                $line =~ s/\n//g;               #remove newline

                $dnaseq{ $counter } = {
                        line => $line,
                        newid => $id,
                };

                $sequence = "";
        } else {
                $line =~ s/\n//g;               #remove newline
                $sequence = $sequence . $line;
                $dnaseq{ $counter }{sequence} = $sequence;
        }
}

close $dnafile;

if ($aacounter != $counter) {
	die "Error: aa and cds files do not have the same number of sequences!";
}

#### write new files with new id and mapping file
my $newfile_aa = $filename . "_aa_mapped.fa";
my $mapfile_aa = $filename . "_aa.map";
my $newfile_dna = $filename . "_cds_mapped.fa";
my $mapfile_dna = $filename . "_cds.map";

open(my $fh_aa, '>', "$newfile_aa");
open(my $fh_aa_map, '>', "$mapfile_aa");
open(my $fh_dna, '>', "$newfile_dna");
open(my $fh_dna_map, '>', "$mapfile_dna");

for (my $i=1; $i<=$counter; $i++) {
	# write new aa fasta file with new id and sequence
        print $fh_aa ">$aaseq{$i}{'newid'}\n$aaseq{$i}{'sequence'}\n";
	# write aa mapping with new id and old header
	print $fh_aa_map "$aaseq{$i}{'newid'}\t$aaseq{$i}{'line'}\n";
	# write new dna fasta file with new id and sequence
	print $fh_dna ">$dnaseq{$i}{'newid'}\n$dnaseq{$i}{'sequence'}\n";   
        # write dna mapping with new id and old header
        print $fh_dna_map "$dnaseq{$i}{'newid'}\t$dnaseq{$i}{'line'}\n";
}

close $fh_aa;
close $fh_aa_map;
close $fh_dna;
close $fh_dna_map;
