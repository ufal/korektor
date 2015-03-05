#!/bin/sh

KOREKTOR=../../../src
MORPHODITA=../../../../morphodita/src
SRILM=../../../../srilm

# Download Pride and Prejudice from Gutenberg
curl -L http://www.gutenberg.org/ebooks/1342.txt.utf-8 | sed '1,/^PRIDE/{/^PRIDE/p;d}; /^End of the Project Gutenberg EBook/Q' >pride_prejudice.txt

# Tokenize to horizontal format
$MORPHODITA/run_tokenizer --tokenizer=english --output=vertical pride_prejudice.txt | sed '/^./{H;d}; /^$/{x; s/\n/ /g; s/^ */ /; s/ *$/ /; s/ _ / /g; s/^ *//; s/ *$//; }' >pride_prejudice_tokenized.txt

# Split into training and testing data
LINES=`wc -l <pride_prejudice_tokenized.txt`
head -`expr $LINES '*' 9 / 10` pride_prejudice_tokenized.txt >pride_prejudice_tokenized_train.txt
tail -`expr $LINES / 10` pride_prejudice_tokenized.txt >pride_prejudice_tokenized_test.txt
rm pride_prejudice_tokenized.txt

# Run tagger on input data
sed 's/$/\n/; s/ /\n/g' pride_prejudice_tokenized_train.txt | $MORPHODITA/run_tagger --input=vertical --output=vertical $MORPHODITA/../support/models/english-morphium-wsj-140407-no_negation.tagger >pride_prejudice_tokenized_train_tagged.txt

# Replace words with count <= 2 with <unk>
perl -e '
  while (<>) {
    chomp;
    my @parts = split/\t/;
    @parts == 0 or @parts == 3 or die;
    push @input, [@parts];
    $counts{lc($parts[0])}++ if @parts == 3;
  }

  foreach my $i_ref (@input) {
    print join("|", !@{$i_ref} || $counts{lc($i_ref->[0])} > 2 ? @{$i_ref} : ("<unk>")x3) . "\n";
  }
' <pride_prejudice_tokenized_train_tagged.txt >pride_prejudice_tokenized_train_tagged_unk.txt

# Generate morphology
grep -v '^$' <pride_prejudice_tokenized_train_tagged_unk.txt | sort -u | sed '1iform|lemma|tag' | sed '2i-----' >../morphology.txt
grep -v '^$' <pride_prejudice_tokenized_train_tagged_unk.txt | sort | uniq -c | awk '{print $2" "$1}' >../morphology_counts.txt

# Generate LMs
generate_lm() {
  cut -d"|" -f$1 <pride_prejudice_tokenized_train_tagged_unk.txt | sed '/^./{H;d};x;s/^\n//;s/\n/ /g' >pride_prejudice_tokenized_train_lm_$2.txt
  case $2 in
    tag) SRILM_PARAMS="-wbdiscount";;
    *) SRILM_PARAMS="-kndiscount -gt2min 2 -gt3min 2";;
  esac
  $SRILM/ngram-count -order 3 $SRILM_PARAMS -unk -text pride_prejudice_tokenized_train_lm_$2.txt -lm ../lm_$2.lm
  perl -pe 's/(\d\.\d\d\d)\d+/$1/g' -i ../lm_$2.lm
}
generate_lm 1 form
generate_lm 2 lemma
generate_lm 3 tag

# Create test and gold data
cp pride_prejudice_tokenized_test.txt ../test.txt.gold
perl -pe '
  BEGIN{ srand(42); }
  chomp;
  $l = "";
  foreach $c (split //) {
    if (int(rand(100)) || $c !~ /[a-zA-Z]/) {
      $l .= $c;
    } else {
      my $op = int(rand(5));
      $l .= (lc($c) ne $c ? lc($c) : uc($c)) if $op == 0;
      $l .= chr(ord(int(rand(2))?"A":"a")+int(rand(26))) . $c if $op == 1;
      $l .= "" if $op == 2;
      $l .= chr(ord(int(rand(2))?"A":"a")+int(rand(26))) if $op == 3;
      $l = substr($l,0,length($l)-1).$c.substr($l,-1) if $op == 4 && substr($l,-1) =~ /[a-zA-Z]/;
    }
  }
  $_ = $l."\n";
' <pride_prejudice_tokenized_test.txt >../test.txt

rm pride_prejudice*.txt
