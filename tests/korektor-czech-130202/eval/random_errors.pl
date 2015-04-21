#!/usr/bin/perl
use warnings;
use strict;
use utf8;
use open qw(:std :utf8);

srand(42);

while(<>) {
  chomp;
  my $word = "";
  foreach my $c (split //) {
    if (int(rand(100)) || $c !~ /[a-zA-Z]/) {
      $word .= $c;
    } else {
      my $op = int(rand(5));
      $word .= (lc($c) ne $c ? lc($c) : uc($c)) if $op == 0;
      $word .= chr(ord(int(rand(2))?"A":"a")+int(rand(26))) . $c if $op == 1;
      $word .= "" if $op == 2;
      $word .= chr(ord(int(rand(2))?"A":"a")+int(rand(26))) if $op == 3;
      $word = substr($word,0,length($word)-1).$c.substr($word,-1) if $op == 4 && substr($word,-1) =~ /[a-zA-Z]/;
    }
  }
  print $word . "\n";
}
