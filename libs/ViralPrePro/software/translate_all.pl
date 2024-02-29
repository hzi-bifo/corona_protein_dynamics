#!/bin/perl
# This script takes a fasta file with nucleotide sequences and translates them into amino acid sequences using all three forward and all three reverse frames. Results are written onto the console.
# Input files:
# 1. fasta file with nucleotide sequences
use Bio::SeqIO;
use strict;
use warnings;

my $fastafile = $ARGV[0];
#my $outfile = $ARGV[1];

my $in  = Bio::SeqIO->new(-file   => $fastafile ,
                         -format => 'fasta');
my $out = Bio::SeqIO->new(-fh   => \*STDOUT ,
                         -format => 'fasta');

while ( my $seq_obj = $in->next_seq() ) {
	# get current id
	my $id = $seq_obj->display_id;
	# create new ids
	my $newid1= $id . "_1";
	my $newid2= $id . "_2";
	my $newid3= $id . "_3";
        my $newid1R= $id . "_1R";
        my $newid2R= $id . "_2R";
        my $newid3R= $id . "_3R";
	# adjust id and translate sequences in all three forward frames
	$seq_obj->display_id($newid1);
	$out->write_seq($seq_obj->translate(-frame => 0));
	$seq_obj->display_id($newid2);
	$out->write_seq($seq_obj->translate(-frame => 1));
	$seq_obj->display_id($newid3);
	$out->write_seq($seq_obj->translate(-frame => 2));
	# adjust id and translate sequences in all three reverse frames
	$seq_obj->display_id($newid1R);
        $out->write_seq($seq_obj->revcom()->translate(-frame => 0));
        $seq_obj->display_id($newid2R);
        $out->write_seq($seq_obj->revcom()->translate(-frame => 1));
        $seq_obj->display_id($newid3R);
        $out->write_seq($seq_obj->revcom()->translate(-frame => 2));
}
