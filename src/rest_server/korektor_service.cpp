// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include "korektor_service.h"
#include "spellchecker/configuration.h"
#include "spellchecker/spellchecker.h"
#include "token/input_format.h"
#include "token/output_format.h"
#include "token/tokenizer.h"
#include "unilib/unistrip.h"
#include "utils/utf.h"

namespace ufal {
namespace korektor {

#if 0
// Custom ResponseGenerator using ufal::microrestd::json_builder as data storage.
// Pointer to input data is used and additional logic for generate
// is implemented.
class KorektorResponseGenerator : public ResponseGenerator {
 public:
  KorektorResponseGenerator(const string& model, const char* data);

  virtual bool next() = 0;

  virtual ufal::microrestd::string_piece current() override;
  virtual void consume(size_t length) override;
  virtual bool generate() override;

 protected:
  const char* data;
  ufal::microrestd::json_builder result;
};

KorektorResponseGenerator::KorektorResponseGenerator(const string& model, const char* data) : data(data) {
  result.object().key("model").value(model).key("result");
}

ufal::microrestd::string_piece KorektorResponseGenerator::current() {
  return result.current();
}

void KorektorResponseGenerator::consume(size_t length) {
  result.discard_prefix(length);
}

bool KorektorResponseGenerator::generate() {
  if (!data) return false;

  if (!next()) {
    result.close_all();
    data = nullptr;
  }
  return true;
}
#endif

// Spellchecker implementation using Korektor
class KorektorService::KorektorProvider : public KorektorService::SpellcheckerProvider {
 public:
  KorektorProvider(const string& file) : configuration(new Configuration(file)) {}

  class KorektorSpellchecker : public SpellcheckerI {
   public:
    KorektorSpellchecker(Configuration* configuration) : spellchecker(configuration) {}

    virtual void Suggestions(const vector<TokenP>& tokens, vector<SpellcheckerCorrection>& corrections, unsigned alternatives) override {
      spellchecker.Spellcheck(tokens, corrections, alternatives);
    }
   private:
    Spellchecker spellchecker;
  };

  virtual SpellcheckerI* NewSpellchecker() const override {
    return new KorektorSpellchecker(configuration.get());
  }
  virtual LexiconP Lexicon() const override {
    return configuration->lexicon;
  }
 private:
  unique_ptr<Configuration> configuration;
};

// Spellchecker implementation stripping diacritics
class KorektorService::StripDiacriticsProvider : public KorektorService::SpellcheckerProvider {
 public:
  class StripDiacriticsSpellchecker : public SpellcheckerI {
   public:
    virtual void Suggestions(const vector<TokenP>& tokens, vector<SpellcheckerCorrection>& corrections, unsigned /*alternatives*/) override {
      corrections.clear();
      for (auto&& token : tokens) {
        bool contains_diacritics = false;
        for (auto&& chr : token->str)
          if (unilib::unistrip::is_combining_mark(chr) || unilib::unistrip::strip_combining_marks(chr) != chr) {
            contains_diacritics = true;
            break;
          }

        corrections.emplace_back(contains_diacritics ? SpellcheckerCorrection::SPELLING : SpellcheckerCorrection::NONE);
        if (contains_diacritics)
          for (auto&& chr : token->str)
            if (!unilib::unistrip::is_combining_mark(chr))
              corrections.back().correction.push_back(unilib::unistrip::strip_combining_marks(chr));
      }
    }
  };

  virtual SpellcheckerI* NewSpellchecker() const override {
    return new StripDiacriticsSpellchecker();
  }
  virtual LexiconP Lexicon() const override {
    return nullptr;
  }
};

// Init the Korektor service -- load the spellcheckers
void KorektorService::Init(const vector<SpellcheckerDescription>& spellchecker_descriptions) {
  static const string strip_diacritics = "strip_diacritics";

  // Load spellcheckers
  spellcheckers.clear();
  json_models.clear().object().indent().key("models").indent().array();
  for (auto& spellchecker_description : spellchecker_descriptions) {
    spellcheckers.emplace(spellchecker_description.id, unique_ptr<SpellcheckerProvider>(new KorektorProvider(spellchecker_description.file)));
    json_models.indent().value(spellchecker_description.id);
  }
  spellcheckers.emplace(strip_diacritics, unique_ptr<SpellcheckerProvider>(new StripDiacriticsProvider()));
  json_models.indent().value(strip_diacritics);

  default_model = spellchecker_descriptions.empty() ? strip_diacritics : spellchecker_descriptions.front().id;
  json_models.indent().close().indent().key("default_model").indent().value(default_model).finish(true);
}

// Handle a request
bool KorektorService::handle(ufal::microrestd::rest_request& req) {
  if (req.url == "/models") return req.respond(json_models.mime, json_models);
  if (req.url == "/correct") return HandleCorrect(req);
  if (req.url == "/suggestions") return HandleSuggestions(req);
  return req.respond_not_found();
}

bool KorektorService::HandleSuggestions(ufal::microrestd::rest_request& req) {
#if 0
  ufal::microrestd::json_builder error;
  auto data = get_data(req, error); if (!data) return req.respond_json(error);
  string model;
  auto provider = get_provider(req, model, error); if (!provider) return req.respond_json(error);
  auto suggestions = get_suggestions(req, error); if (!suggestions) return req.respond_json(error);

   class generator : public KorektorResponseGenerator {
   public:
    generator(const string& model, const char* data, unsigned num, SpellcheckerI* spellchecker)
        : KorektorResponseGenerator(model, data), num(num), spellchecker(spellchecker) { result.array(); }

    bool next() {
      ufal::microrestd::string_piece line;
      if (!get_line(data, line)) return false;

      spellchecker->suggestions(string(line.str, line.len), num, suggestions);
      for (auto&& suggestion : suggestions) {
        result.array().value(suggestion.first);
        for (auto&& word : suggestion.second)
          result.value(word);
        result.close();
      }

      return true;
    }

   private:
    unsigned num;
    unique_ptr<SpellcheckerI> spellchecker;
    vector<pair<string, vector<string>>> suggestions;
  };
  return req.respond_json(new generator(model, data, suggestions, provider->new_spellchecker()));
#endif
}

bool KorektorService::HandleCorrect(ufal::microrestd::rest_request& req) {
  string error;
  auto data = GetData(req, error); if (!data) return req.respond_error(error);
  string model;
  auto provider = GetProvider(req, model, error); if (!provider) return req.respond_error(error);
  auto input_format = InputFormat::NewInputFormat(GetInputFormat(req), provider->Lexicon());

  class Generator : public ufal::microrestd::json_response_generator {
   public:
    Generator(const string& model, const string& data, InputFormat* input_format, SpellcheckerI* spellchecker)
        : input_format(input_format), output_format(OutputFormat::NewOriginalOutputFormat()), spellchecker(spellchecker) {
      input_format->SetBlock(data);
      output_format->SetBlock(data);
      json.object().indent().key("model").indent().value(model).indent().key("result").indent().value("");
    }

    bool generate() override {
      if (!input_format->NextSentence(tokens)) {
        corrected_sentence.clear();
        output_format->FinishBlock(corrected_sentence);
        json.value(corrected_sentence, true).finish(true);
        return false;
      }

      spellchecker->Suggestions(tokens, corrections, 0);

      corrected_sentence.clear();
      output_format->AppendSentence(corrected_sentence, tokens, corrections);
      json.value(corrected_sentence, true);

      return true;
    }

   private:
    unique_ptr<InputFormat> input_format;
    unique_ptr<OutputFormat> output_format;
    unique_ptr<SpellcheckerI> spellchecker;
    vector<TokenP> tokens;
    vector<SpellcheckerCorrection> corrections;
    string corrected_sentence;
  };
  return req.respond(ufal::microrestd::json_builder::mime, new Generator(model, *data, input_format.release(), provider->NewSpellchecker()));
}

const string* KorektorService::GetData(ufal::microrestd::rest_request& req, string& error) {
  auto data_it = req.params.find("data");
  if (data_it == req.params.end()) return error.assign("Required argument 'data' is missing."), nullptr;

  return &data_it->second;
}

const KorektorService::SpellcheckerProvider* KorektorService::GetProvider(ufal::microrestd::rest_request& req, string& model, string& error) {
  auto data_it = req.params.find("model");
  model.assign(data_it == req.params.end() ? default_model : data_it->second);

  auto spellchecker_it = spellcheckers.find(model);
  if (spellchecker_it == spellcheckers.end()) return error.assign("Specified model '").append(model).append("' does not exist."), nullptr;

  return spellchecker_it->second.get();
}

const string& KorektorService::GetInputFormat(ufal::microrestd::rest_request& req) {
  static const string untokenized = "untokenized";

  auto data_it = req.params.find("input");
  return data_it != req.params.end() ? data_it->second : untokenized;
}

unsigned KorektorService::GetSuggestions(ufal::microrestd::rest_request& req, string& error) {
  auto data_it = req.params.find("suggestions");
  if (data_it == req.params.end()) return 5;

  int suggestions = atoi(data_it->second.c_str());
  if (suggestions <= 0) return error.assign("Specified number of suggestions '").append(data_it->second).append("' is not a positive integer."), 0;

  return suggestions;
}

} // namespace korektor
} // namespace ufal
