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
bool KorektorService::Init(const vector<SpellcheckerDescription>& spellchecker_descriptions) {
  static const string strip_diacritics = "strip_diacritics-130202";

  // Load spellcheckers
  spellcheckers.clear();
  spellcheckers_map.clear();
  for (auto& spellchecker_description : spellchecker_descriptions)
    spellcheckers.emplace_back(spellchecker_description.id, spellchecker_description.acknowledgements, new KorektorProvider(spellchecker_description.file));
  spellcheckers.emplace_back(strip_diacritics, string(), new StripDiacriticsProvider());

  // Fill spellcheckers_map with model name and aliases
  for (auto& spellchecker : spellcheckers) {
    // Fail if this spellchecker id is aready in use.
    if (!spellcheckers_map.emplace(spellchecker.id, &spellchecker).second) return false;

    // Create (but not overwrite) id without version.
    for (unsigned i = 0; i+1+6 < spellchecker.id.size(); i++)
      if (spellchecker.id[i] == '-') {
        bool is_version = true;
        for (unsigned j = i+1; j < i+1+6; j++)
          is_version = is_version && spellchecker.id[j] >= '0' && spellchecker.id[j] <= '9';
        if (is_version)
          spellcheckers_map.emplace(spellchecker.id.substr(0, i) + spellchecker.id.substr(i+1+6), &spellchecker);
      }

    // Create (but not overwrite) hyphen-separated prefixes.
    for (unsigned i = 0; i < spellchecker.id.size(); i++)
      if (spellchecker.id[i] == '-')
        spellcheckers_map.emplace(spellchecker.id.substr(0, i), &spellchecker);
  }
  spellcheckers_map.emplace(string(), &spellcheckers.front());

  // Fill json_models
  json_models.clear().object().indent().key("models").indent().array();
  for (auto& spellchecker : spellcheckers)
    json_models.indent().value(spellchecker.id);
  json_models.indent().close().indent().key("default_model").indent().value(spellcheckers.front().id).finish(true);

  return true;
}

// Load selected model
const KorektorService::SpellcheckerModel* KorektorService::LoadSpellchecker(const string& id, string& error) {
  auto model_it = spellcheckers_map.find(id);
  if (model_it == spellcheckers_map.end())
    return error.assign("Requested model '").append(id).append("' does not exist.\n"), nullptr;

  return model_it->second;
}

// Handle a request
unordered_map<string, bool (KorektorService::*)(ufal::microrestd::rest_request&)> KorektorService::handlers = {
  {"/models", &KorektorService::HandleModels},
  {"/correct", &KorektorService::HandleCorrect},
  {"/suggestions", &KorektorService::HandleSuggestions},
};

bool KorektorService::handle(ufal::microrestd::rest_request& req) {
  auto handler_it = handlers.find(req.url);
  return handler_it == handlers.end() ? req.respond_not_found() : (this->*handler_it->second)(req);
}

// REST service
bool KorektorService::HandleModels(ufal::microrestd::rest_request& req) {
  return req.respond(json_models.mime, json_models);
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
  auto model = LoadSpellchecker(GetModelId(req), error); if (!model) return req.respond_error(error);
  auto input_format = InputFormat::NewInputFormat(GetInputFormat(req), model->spellchecker->Lexicon());

  class Generator : public ufal::microrestd::json_response_generator {
   public:
    Generator(const SpellcheckerModel* model, const string& data, InputFormat* input_format)
        : input_format(input_format), output_format(OutputFormat::NewOriginalOutputFormat()), spellchecker(model->spellchecker->NewSpellchecker()) {
      input_format->SetBlock(data);
      output_format->SetBlock(data);
      CommonResponse(model, json);
      json.value("");
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
  return req.respond(ufal::microrestd::json_builder::mime, new Generator(model, *data, input_format.release()));
}

void KorektorService::CommonResponse(const SpellcheckerModel* model, ufal::microrestd::json_builder& json) {
  json.object();
  json.indent().key("model").indent().value(model->id);
  json.indent().key("acknowledgements").indent().array();
  json.indent().value("http://ufal.mff.cuni.cz/korektor#korektor_acknowledgements");
  if (!model->acknowledgements.empty()) json.indent().value(model->acknowledgements);
  json.indent().close().indent().key("result").indent();
}

const string* KorektorService::GetData(ufal::microrestd::rest_request& req, string& error) {
  auto data_it = req.params.find("data");
  if (data_it == req.params.end()) return error.assign("Required argument 'data' is missing.\n"), nullptr;

  return &data_it->second;
}

const string& KorektorService::GetModelId(ufal::microrestd::rest_request& req) {
  static const string empty;

  auto data_it = req.params.find("model");
  return data_it != req.params.end() ? data_it->second : empty;
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
  if (suggestions <= 0) return error.assign("Specified number of suggestions '").append(data_it->second).append("' is not a positive integer.\n"), 0;

  return suggestions;
}

} // namespace korektor
} // namespace ufal
