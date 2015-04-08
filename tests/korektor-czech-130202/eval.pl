#!/usr/bin/perl
use warnings;
use strict;
use utf8;
use open qw(:std :utf8);

@ARGV >= 3 || die "Usage: $0 korektor_output_file gold_file nbest\n";
my $korektor = shift @ARGV;
my $gold = shift @ARGV;
my $nbest = shift @ARGV;

my @input;
my @corrections;
open(my $f, "<", $korektor) or die "Cannot open file '$korektor': $!";
while (<$f>) {
  chomp;
  next if /^$/;
  my @parts = split /\t/, $_;
  push @input, $parts[0];
  push @corrections, [@parts[2..$#parts]];
}
close $f;

my @gold;
open($f, "<", $gold) or die "Cannot open file '$gold': $!";
while (<$f>) {
  chomp;
  next if /^$/;
  push @gold, $_;
}
close $f;

die "Different word counts of korektor_output_file and gold_file" unless $#input == $#gold;

my %detection_counts = (TP=>0, TN=>0, FP=>0, FN=>0); # rror detection
my @correction_counts = map {{TP=>0, TN=>0, FP=>0, FN=>0}} (1..$nbest); #top-n error correction

for (my $i = 0; $i < @input; $i++) {
  # Detection
  my $key = $input[$i] ne $gold[$i] ? (@{$corrections[$i]} ? "TP" : "FN") : (@{$corrections[$i]} ? "FP" : "TN");
  $detection_counts{$key}++;

  # Correction
  my $correct = 0;
  for (my $j = 0; $j < $nbest; $j++) {
    $correct = $correct || (@{$corrections[$i]} > $j && $corrections[$i]->[$j] eq $gold[$i]);
    $key = $input[$i] ne $gold[$i] ? (@{$corrections[$i]} ? ($correct ? "TP" : "FP") : "FN") : (@{$corrections[$i]} ? "FP" : "TN");
    $correction_counts[$j]->{$key}++;
  }
}

printf "               |   A   |   P   |   R   |   F   |\n";
foreach my $set ( {name=>"Detection",data=>\%detection_counts},
                  map {{name=>"Correct ".$_."-best",data=>$correction_counts[$_-1]}} (1..$nbest)
                ) {
  my $d = $set->{data};
  printf "%-14s | %5.2f |", $set->{name}, 100*($d->{TN}+$d->{TP})/@input;
  if (!$d->{TP}) {
    printf "%s\n", "       |"x3;
  } else {
    printf " %5.2f | %5.2f | %5.2f |\n", 100*$d->{TP}/($d->{TP}+$d->{FP}), 100*$d->{TP}/($d->{TP}+$d->{FN}), 100*2*$d->{TP}/(2*$d->{TP}+$d->{FP}+$d->{FN});
  }
}
