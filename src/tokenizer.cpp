// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "common.h"
#include "token/input_format.h"
#include "token/output_format.h"
#include "utils/options.h"
#include "version/version.h"

using namespace ufal::korektor;

int main(int argc, char* argv[]) {
  iostream::sync_with_stdio(false);

  Options::Map options;
  if (!Options::Parse({{"input",Options::Value{"untokenized", "untokenized_lines", "segmented", "vertical", "horizontal"}},
                      {"output",Options::Value{"vertical", "horizontal"}},
                      {"version", Options::Value::none},
                      {"help", Options::Value::none}}, argc, argv, options) ||
      options.count("help"))
    runtime_failure("Usage: " << argv[0] << " [options]\n"
                    "Options: --input=untokenized|untokenized_lines|segmented|vertical|horizontal\n"
                    "         --output=vertical|horizontal\n"
                    "         --version\n"
                    "         --help");
  if (options.count("version"))
    return cout << version::version_and_copyright() << endl, 0;

  // Init input format
  string input_format_name = options.count("input") ? options["input"] : "untokenized";
  auto input_format = InputFormat::NewInputFormat(input_format_name, nullptr);
  if (!input_format)
    runtime_failure("Cannot create input format '" << input_format_name << "'.");

  // Init output format
  string output_format_name = options.count("output") ? options["output"] : "horizontal";
  auto output_format = OutputFormat::NewOutputFormat(output_format_name);
  if (!output_format)
    runtime_failure("Cannot create output format '" << output_format_name << "'.");

  // Process data
  string input_block, output_block;
  vector<TokenP> tokens;
  vector<SpellcheckerCorrection> corrections;
  while (input_format->ReadBlock(cin, input_block)) {
    input_format->SetBlock(input_block);
    output_format->SetBlock(input_block);
    output_block.clear();

    // Process all sentences in the input block
    while (input_format->NextSentence(tokens)) {
      corrections.resize(tokens.size(), SpellcheckerCorrection(SpellcheckerCorrection::NONE));
      // Output the results
      output_format->AppendSentence(output_block, tokens, corrections);
    }

    // Write the output block
    output_format->FinishBlock(output_block);
    cout << output_block;
  }

  return 0;
}
