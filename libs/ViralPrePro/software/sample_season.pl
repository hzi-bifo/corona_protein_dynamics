#!/bin/perl
#####
# this script takes amino acid and nucleotide sequence fasta files and samples a specified number of sequences per season
# arguments are the name of the files, the number of sequences to be sampled and the root strain
# the beginning of the fasta header after > should contain an ID
# the date has to be provided in the fasta header in the format YYYY/MM/DD or YYYY-MM-DD
# the root strain should be specified with the isolate name
#####
use strict;
use warnings;

my $filename = $ARGV[0];
my $number_seq = $ARGV[1];
my $root = $ARGV[2];

# set filenames
my $aafilename = $filename . "_aa.fa";
my $dnafilename = $filename . "_cds.fa";

# open fasta file with amino acid sequences
open (my $aafile, $aafilename) or die "Could not open file '$aafilename' $!";

#### parse amino acid file into hash and infer seasons at the same time
my $id = "";
my $sequence = "";
my $date = "";
my $nodate = 0;
my $root_id = "";

my %aaseq = ();
my %seasons = ();

while( my $line = <$aafile>)  {
        if ($line =~ /^>/) {
		#extract date
		$line =~ m/\s([12][0-9]{3}[\/|-][01][0-9][\/|-][0123][0-9])\s/;
		$date = $1;

		#for further processing: remove newline
                $line =~ s/\n//g;

		#omit sequence when date was not found and print error
		if (not defined $date) {
			print "Date not found in amino acid sample $line! Provide date in format YYYY/MM/DD or YYYY-MM-DD\n";
			$nodate = 1;
			next;
		} else {
			$nodate = 0;
		}

		#get month and year from date
		my @splitdate = split /[\/|-]/, $date;
		my $year = $splitdate[0];
		my $month = $splitdate[1];

		#get id
		$line =~ m/>(cds:|EPI)?(\S{1,10})/;
		$id = $2;
		if (not defined $id){
			print "No ID number found after > in amino acid sample $line!\n";
		}

		#get root
		#$line =~ m/\s([ABC][\/][A-Za-z \-_]*[\/]*[A-Za-z \-_]+[\/]\S+[\/][12][0-9]{3})\s/;
		$line =~ m/\s(hCoV-19[\/][A-Za-z \-_]*[\/]*[A-Za-z \-_]+[\/]\S+[\/][12][0-9]{3})\s/;
		my $isolate = $1;

		#check if sequence is root
		if ($isolate eq $root) {
			$root_id = $id;
		}
	
		#save data in hash
                $aaseq{ $id } = {
                        line => $line,
                };

		#get season
		my $season = "";
		my $seasonyear = "";
		if ($month >= 4 && $month <= 9) {
			$season = $year . "S";
		}
		elsif ($month <= 3) {
			$season = $year . "N";
		} elsif ($month >= 10) {
			$seasonyear = $year + 1;
			$season = $seasonyear . "N";
		}

		#save seasons in hash of arrays
		if (exists $seasons{ $season }) {
			push @{ $seasons{ $season } }, "$id";		#append to existing array
		} else {
			$seasons{ $season } = [ "$id" ];		#initialize array
		}

                $sequence = "";
        } else {
		if ($nodate == 1) {
			next;			#omit sequence when date was not found
		}
                $line =~ s/\n//g;               #remove newline
		$line =~ s/\r//g;		#remove dos newline (gisaid data)
                $sequence = $sequence . $line;
                $aaseq{ $id }{sequence} = $sequence;
        }
}

# open fasta file with nucleotide sequences 
open (my $dnafile, $dnafilename) or die "Could not open file '$dnafilename' $!";

# parse data into hash
my %dnaseq = ();

while( my $line = <$dnafile>)  {
        if ($line =~ /^>/) {
                $line =~ s/\n//g;               #remove newline

                #get id
                $line =~ m/>(cds:|EPI)?(\S{1,10})/;
                $id = $2;

                #save data in hash
                $dnaseq{ $id } = {
                        line => $line,
                };

                $sequence = "";
        } else {
                $line =~ s/\n//g;               #remove newline
                $line =~ s/\r//g;               #remove dos newline (gisaid data)
                $sequence = $sequence . $line;
                $dnaseq{ $id }{sequence} = $sequence;
        }
}

# define new files
my $newfile_cds = $filename . "_cds_sampled.fa";
my $newfile_aa = $filename . "_aa_sampled.fa";

open(my $fh_cds, '>', "$newfile_cds");
open(my $fh_aa, '>', "$newfile_aa");

#check if root was found
if ($root_id eq ""){
	#print error
        print "Root $root was not found in file $aafilename.\n";
	exit 2;
} else {
	#sample root first
        print $fh_cds "$dnaseq{$root_id}{'line'}\n$dnaseq{$root_id}{'sequence'}\n";
        print $fh_aa "$aaseq{$root_id}{'line'}\n$aaseq{$root_id}{'sequence'}\n";
}

my $random = "";
foreach my $key ( keys %seasons ) {
	my @array = @{ $seasons{ $key }};
	for (my $i=1; $i <= $number_seq; $i++) {
		$random = $array[rand @array];

       		print $fh_cds "$dnaseq{$random}{'line'}\n$dnaseq{$random}{'sequence'}\n";
        	print $fh_aa "$aaseq{$random}{'line'}\n$aaseq{$random}{'sequence'}\n";
	}
}

close $fh_cds;
close $fh_aa;
