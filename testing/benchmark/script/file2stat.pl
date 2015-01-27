#!/usr/bin/perl

use warnings;
use strict;
use Statistics::PointEstimation;

my $numargs =  $#ARGV + 1;
if ($numargs != 1) {
    print "USAGE: ./data2stat.pl <data file>\n";
    exit 1;
}

my $f_data = $ARGV[0];
my @data = ();
my $i = 0;
my $row;
my $column;
open DATA, "<$f_data" or die $!;
while (<DATA>) {
    next if /^\s*#/; # ignore comments
    next if /^\s*$/; # ignore empty lines
    my @line = split;
    $i=0;
    foreach $column (@line) {
        chomp ($column);
        push (@{$data[$i]}, $column);
        $i++;
    }
}
close (DATA);

my @output = ();
foreach $row (0..@data-1) {
    my $stat   = new Statistics::PointEstimation;
    $stat->set_significance(95);
    $stat->add_data(@{$data[$row]});
    my $mean = $stat->mean();
    push (@output, $mean);
    my $sdev = $stat->standard_deviation();
    push (@output, $sdev);    
}
my $c;
foreach $c (@output) {
    print "$c ";
}
print "\n";
