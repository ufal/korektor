// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "common.h"
#include "lexicon/lexicon.h"
#include "spellchecker/configuration.h"
#include "spellchecker/spellchecker.h"
#include "token/input_format.h"
#include "token/output_format.h"
#include "utils/options.h"
#include "utils/parse.h"
#include "version/version.h"

using namespace ufal::korektor;

int main(int argc, char* argv[]) {
  iostream::sync_with_stdio(false);

  Options::Map options;
  if (!Options::Parse({{"input",Options::Value{"untokenized", "untokenized_lines", "segmented", "vertical", "horizontal"}},
                       {"output",Options::Value{"original", "xml", "vertical", "horizontal"}},
                       {"corrections",Options::Value::any},
                       {"context_free", Options::Value::none},
                       {"viterbi_order", Options::Value::any},
                       {"viterbi_beam_size", Options::Value::any},
                       {"viterbi_stage_pruning", Options::Value::any},
                       {"version", Options::Value::none},
                       {"help", Options::Value::none}}, argc, argv, options) ||
      options.count("help") ||
      (argc < 2 && !options.count("version")))
    runtime_failure("Usage: " << argv[0] << " [options] model_configuration\n"
                    "Options: --input=untokenized|untokenized_lines|segmented|vertical|horizontal\n"
                    "         --output=original|xml|vertical|horizontal\n"
                    "         --corrections=maximum_number_of_corrections\n"
                    "         --viterbi_order=viterbi_decoding_order\n"
                    "         --viterbi_beam_size=maximum_viterbi_beam_size\n"
                    "         --viterbi_stage_pruning=maximum_viterbi_stage_cost_increment\n"
                    "         --context_free\n"
                    "         --version\n"
                    "         --help");
  if (options.count("version"))
    return cout << version::version_and_copyright() << endl, 0;

  // Init options
  unsigned max_corrections = 1;
  if (options.count("corrections")) {
    max_corrections = Parse::Int(options["corrections"], "maximum number of corrections");
    if (max_corrections <= 0)
      runtime_failure("The maximum number of corrections (" << max_corrections << ") must be at least 1!");
  }

  unsigned viterbi_order = 0;
  if (options.count("viterbi_order")) {
    int order = Parse::Int(options["viterbi_order"], "viterbi order");
    if (order <= 0) runtime_failure("Specified viterbi order '" << order << "' must be greater than zero!");
    viterbi_order = order;
  }

  unsigned viterbi_beam_size = 0;
  if (options.count("viterbi_beam_size")) {
    int beam_size = Parse::Int(options["viterbi_beam_size"], "viterbi beam size");
    if (beam_size <= 0) runtime_failure("Specified viterbi beam size '" << beam_size << "' must be greater than zero!");
    viterbi_beam_size = beam_size;
  }

  double viterbi_stage_pruning = 0.;
  if (options.count("viterbi_stage_pruning")) {
    double pruning = Parse::Double(options["viterbi_stage_pruning"], "viterbi stage pruning");
    if (pruning <= 0.) runtime_failure("Specified viterbi stage pruning '" << pruning << "' must be greater than zero!");
    viterbi_stage_pruning = pruning;
  }

  bool context_free = options.count("context_free");

  // Load spellchecker
  ConfigurationP configuration(new Configuration(argv[1]));
  if (viterbi_order) configuration->viterbi_order = viterbi_order;
  if (viterbi_beam_size) configuration->viterbi_beam_size = viterbi_beam_size;
  if (viterbi_stage_pruning) configuration->viterbi_stage_pruning = viterbi_stage_pruning;
  Spellchecker spellchecker = Spellchecker(configuration.get());

  // Init input format
  string input_format_name = options.count("input") ? options["input"] : "untokenized";
  auto input_format = InputFormat::NewInputFormat(input_format_name, configuration->lexicon);
  if (!input_format)
    runtime_failure("Cannot create input format '" << input_format_name << "'.");

  // Init output format
  string output_format_name = options.count("output") ? options["output"] : max_corrections == 1 ? "original" : "xml";
  auto output_format = OutputFormat::NewOutputFormat(output_format_name);
  if (!output_format)
    runtime_failure("Cannot create output format '" << output_format_name << "'.");

  // Check that output format supports multiple corrections if required
  if (max_corrections > 1 && !output_format->CanHandleAlternatives())
    runtime_failure("Output format '" << output_format_name << "' cannot handle multiple corrections!");

  // Process data
  string input_block, output_block;
  vector<TokenP> tokens;
  vector<SpellcheckerCorrection> corrections;
  while (input_format->ReadBlock(cin, input_block)) {
    input_format->SetBlock(input_block);
    output_format->SetBlock(input_block);
    output_block.clear();

    // For all sentences in the input block
    while (input_format->NextSentence(tokens)) {
      // Perform the spellchecking
      if (!context_free) {
        spellchecker.Spellcheck(tokens, corrections, max_corrections - 1);
      } else {
        corrections.resize(tokens.size());
        for (unsigned i = 0; i < tokens.size(); i++)
          spellchecker.SpellcheckToken(tokens[i], corrections[i], max_corrections - 1);
      }

      // Output the results
      output_format->AppendSentence(output_block, tokens, corrections);
    }

    // Write the output block
    output_format->FinishBlock(output_block);
    cout << output_block;
  }

  return 0;
}
