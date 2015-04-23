#!/usr/bin/perl
use warnings;
use strict;
use utf8;
use open qw(:std :utf8);

@ARGV >= 1 or die "Usage: $0 maximum_edit_distance\n";
my $edit_distance = shift @ARGV;

srand(42);

while(<>) {
  chomp;
  my $word = $_;
  if (int(rand(100)) <= 4) {
    for (my $i = int(rand($edit_distance)); $i >= 0; $i--) {
      my $j = int(rand(length $word));
      next unless substr($word,$j) =~ /\w/;
      my $op = int(rand(5));
      substr $word, $j, 1, lc substr($word,$j) ne substr($word,$j) ? lc substr($word,$j) : uc substr($word,$j) if $op == 0;
      substr $word, $j, 0, chr(ord(int(rand(2))?"A":"a")+int(rand(26))) if $op == 1;
      substr $word, $j, 1, "" if $op == 2;
      substr $word, $j, 1, chr(ord(int(rand(2))?"A":"a")+int(rand(26))) if $op == 3;
      substr $word, $j, 2, substr($word,$j+1,1).substr($word,$j,1) if $op == 4 && substr($word,$j,2) =~ /\w\w/;
    }
  }
  print $word . "\n";
}
