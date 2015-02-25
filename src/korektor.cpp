// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

/// @file korektor.cpp
/// @brief Main Korektor executable
///
/// usage: ./korektor configuration_file <input_text >output_text

#include "common.h"
#include "korlib/configuration.h"
#include "korlib/lexicon.h"
#include "korlib/spellchecker.h"
#include "korlib/tokenizer.h"
#include "korlib/utils.h"

/// @brief Help message
void print_help()
{
  cerr << "usage: ./korektor configuration_file <input_text >output_text" << endl;
}

/// @brief Main entry point
int main(int argc, char** argv)
{
  if (argc < 2)
  {
    print_help();
    exit(1);
  }


  ConfigurationP configuration(new Configuration(argv[1]));
  Spellchecker spellchecker = Spellchecker(configuration.get());

  //uint range_from, range_length;
  //spellchecker.FindMisspelledWord("mrkev brambora pstrah", range_from, range_length);

  //vector<TextCheckingResultP> result = spellchecker.GetCheckingResultsFirstSentence("Podlaha je pstrah. To je dobre\nFakt!", range_from, range_length);

  //vector<string> suggestions = spellchecker.GetContextFreeSuggestions("pstryh");

  string s;
  string out;

  if (argc >= 4 && strcmp(argv[2], "-gold") == 0)
  {
    ifstream gold_ifs(argv[3]);
    if (gold_ifs.is_open() == false)
    {
      cerr << "Can't open gold data file!" << endl;
      exit(1);
    }

    uint fail = 0;
    uint success = 0;

    string gold_s;

    uint counter = 0;

    while (MyUtils::SafeReadline(cin, s))
    {
      counter++;

      if (counter % 100 == 0) cerr << counter << endl;

      if (gold_ifs.eof())
      {
        cerr << "Error: Gold data have fewer lines than input data!" << endl;
        exit(1);
      }

      MyUtils::SafeReadline(gold_ifs, gold_s);

      out = spellchecker.CheckText(s);

      vector<vector<TokenP>> out_toks = configuration->tokenizer->Tokenize(MyUtils::utf8_to_utf16(out));
      vector<vector<TokenP>> gold_toks = configuration->tokenizer->Tokenize(MyUtils::utf8_to_utf16(gold_s));

      if (out_toks.size() != gold_toks.size())
      {
        cerr << "Error: input line splitted into different numbers of sentences in input data and gold data!" << endl;
        cerr << "output: " << out << endl << endl;
        cerr << "gold: " << gold_s << endl << endl;
        exit(1);
      }

      for (uint i = 0; i < out_toks.size(); i++)
      {
        vector<TokenP> &out_t = out_toks[i];
        vector<TokenP> &gold_t = gold_toks[i];

        if (out_t.size() != gold_t.size())
        {
          cerr << "warning: different number of tokens per sentence on line '" << s << "' ! Sentence skipped!" << endl;
          continue;
        }

        bool contains_error = false;
        for (uint j = 0; j < out_t.size(); j++)
        {
          uint gold_wid = configuration->lexicon->GetWordID(gold_t[j]->str_u16);

          //specific to current lexicon! (28 special words - punctuation and so - we don't care about these)
          if (gold_wid >= 28)
          {
            if (gold_t[j]->str_utf8 == out_t[j]->str_utf8)
              success++;
            else
            {
              contains_error = true;
              //failures.push_back("GOLD: " + gold_t[j]->str_utf8 + " - OUT: " + out_t[j]->str_utf8);
              fail++;
            }
          }
        }

        if (contains_error)
        {
          cout << "GOLD:";
          for (uint j = 0; j < gold_t.size(); j++)
            cout << " " << gold_t[j]->str_utf8;
          cout << "\nOUT:";
          for (uint j = 0; j < out_t.size(); j++)
            cout << " " << out_t[j]->str_utf8;
          cout << endl << endl;
        }
      }
    }

    cout << "accuracy = " << success / (float)(success + fail) << endl;

    return 0;
  }

  while (MyUtils::SafeReadline(cin, s))
  {
    if (configuration->mode_string == "tag_errors")
    {
      out = spellchecker.command_line_mode(s, 5);
    }
    else if (configuration->mode_string == "autocorrect")
    {
      out = spellchecker.CheckText(s);
    }

    cout << out << endl;
  }

  exit(0);
  return 0;
}
