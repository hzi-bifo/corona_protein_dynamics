#!/bin/perl
# this script takes a fasta file and a list of desired IDs and outputs the requested sequences in a new file
# Input:
# 1. a fasta file with sequences
# 2. a file with a sequence ID in each line. IDs should be the string after the > of the fasta header until the first whitespace character.
# 3. a string determining the output file

use strict;
use warnings;
#use Data::Dumper;

my $fastafile = $ARGV[0];
my $list = $ARGV[1];
my $output = $ARGV[2];

open (my $fasta, $fastafile) or die "Could not open file '$fastafile' $!";

my %seq = ();
my $id = "";
my $sequence = "";

# parse fasta file into hash
while( my $line = <$fasta>)  {
        if ($line =~ /^>/) {
                $line =~ s/\n//g;               #remove newline
		
                # get id
                $line =~ m/>(\S+)/;
		if (! defined($1)) {
			print "Error: sequence ID was not found.\n";
		}
                $id = $1;
		
                # save data in hash
                $seq{ $id } = {
                        header => $line,
                };

                $sequence = "";
        } else {
                $line =~ s/\n//g;               #remove newline
                $sequence = $sequence . $line;
                $seq{ $id }{sequence} = $sequence;
        }
}
#print Dumper(\%seq);
close $fasta;

# open list with IDs and new fasta file
open(my $new_fasta, '>', "$output");
open (my $ids, $list) or die "Could not open file '$list' $!";

# loop over all IDs in file
while( my $line = <$ids>) {
        chomp($line);
	# print header and sequence belonging to current ID in new file
	print $new_fasta "$seq{ $line }{'header'}\n$seq{ $line }{'sequence'}\n";
}
close $ids;
close $new_fasta;
