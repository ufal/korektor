// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include <iostream>

#include "common.h"
#include "lexicon/lexicon.h"
#include "spellchecker/configuration.h"
#include "spellchecker/spellchecker.h"
#include "token/token.h"
#include "utils/utils.h"
#include "utils/utf.h"

using namespace ufal::korektor;

int main(int argc, char* argv[])
{
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " configuration_file" << endl;
    return 0;
  }

  // Load the spell checker
  Configuration configuration(argv[1]);
  Spellchecker spellchecker(&configuration);

  // Perform the correction
  vector<pair<string, vector<string>>> suggestions;
  for (bool not_eof = true; not_eof; ) {
    // Read input block
    string line;
    vector<TokenP> tokens;
    vector<string> replaced;
    unsigned para_length = 0;
    while ((not_eof = Utils::SafeReadline(cin, line)) && !line.empty()) {
      string::size_type tab = line.find('\t');
      if (tab == string::npos) {
        replaced.emplace_back();
      } else {
        replaced.emplace_back(line.substr(0, tab));
        line.erase(0, tab + 1);
      }

      u16string word(UTF::UTF8To16(line));

      TokenP token(new Token(para_length, word.size(), word));

      if (configuration.lexicon) {
        int wordID = configuration.lexicon->GetWordID(word);
        token->InitLexiconInformation(wordID, replaced.back().empty() ? configuration.lexicon->CorrectionIsAllowed(wordID) : false);
      }
      token->correction_is_allowed = token->correction_is_allowed && UTF::ContainsLetter(token->str_u16);
      token->sentence_start = tokens.empty();
      tokens.emplace_back(token);

      para_length += word.size() + 1;
    }

    if (!tokens.empty()) {
      // Perform correction
      vector<pair<string, vector<string>>> suggestions;
      spellchecker.GetTokenizedSuggestions(tokens, 5, suggestions);

      // Print results
      for (unsigned i = 0; i < suggestions.size(); i++) {
        if (replaced[i].empty()) {
          cout << suggestions[i].first;
          if (!suggestions[i].second.empty() && !(suggestions[i].second.size() == 1 && suggestions[i].second[0] == suggestions[i].first)) {
            cout << '\t' << (tokens[i]->isUnknown() ? 'S' : 'G');
            for (auto&& word : suggestions[i].second)
              cout << '\t' << word;
          }
        } else {
          cout << replaced[i];
          if (tokens[i]->str_utf8 != replaced[i])
            cout << '\t' << (configuration.lexicon->GetWordID(UTF::UTF8To16(replaced[i])) == -1 ? 'S' : 'G') << '\t' << tokens[i]->str_utf8;
        }

        cout << endl;
      }
    }
    if (not_eof) cout << endl;
  }

  return 0;
}
