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

  static const string model_name;
};
const string KorektorService::StripDiacriticsProvider::model_name = "strip_diacritics-130202";

// Init the Korektor service -- load the spellcheckers
bool KorektorService::Init(const vector<SpellcheckerDescription>& spellchecker_descriptions) {
  // Load spellcheckers
  spellcheckers.clear();
  spellcheckers_map.clear();
  for (auto& spellchecker_description : spellchecker_descriptions)
    spellcheckers.emplace_back(spellchecker_description.id, spellchecker_description.language, spellchecker_description.acknowledgements, new KorektorProvider(spellchecker_description.file));
  spellcheckers.emplace_back(StripDiacriticsProvider::model_name, string(), string(), new StripDiacriticsProvider());

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
  // REST service
  {"/models", &KorektorService::HandleModels},
  {"/correct", &KorektorService::HandleCorrect},
  {"/suggestions", &KorektorService::HandleSuggestions},
  // Weblicht service
  {"/weblicht/spellcheck", &KorektorService::HandleWeblicht},
};

bool KorektorService::handle(ufal::microrestd::rest_request& req) {
  if (req.method != "GET" && req.method != "HEAD" && req.method != "POST") return req.respond_method_not_allowed("GET, HEAD, POST");

  auto handler = handlers.find(req.url);
  return handler == handlers.end() ? req.respond_not_found() : (this->*handler->second)(req);
}

// REST service
bool KorektorService::HandleModels(ufal::microrestd::rest_request& req) {
  return req.respond(json_models.mime, json_models);
}

bool KorektorService::HandleSuggestions(ufal::microrestd::rest_request& req) {
  string error;
  auto data = GetData(req, error); if (!data) return req.respond_error(error);
  auto model = LoadSpellchecker(GetModelId(req), error); if (!model) return req.respond_error(error);
  auto suggestions = GetSuggestions(req, error); if (!suggestions) return req.respond_error(error);
  auto input_format = InputFormat::NewInputFormat(GetInputFormat(req), model->spellchecker->Lexicon());
  if (!input_format) return req.respond_error("Unsupported input format.\n");

  class Generator : public ufal::microrestd::json_response_generator {
   public:
    Generator(const SpellcheckerModel* model, const string& data, InputFormat* input_format, unsigned suggestions)
        : data(UTF::UTF8To16(data)), unprinted(0), input_format(input_format), spellchecker(model->spellchecker->NewSpellchecker()), suggestions(suggestions) {
      input_format->SetBlock(data);
      CommonResponse(model, json);
      json.array();
    }

    bool generate() override {
      if (!input_format->NextSentence(tokens)) {
        if (unprinted < data.size()) {
          next_token.clear();
          UTF::UTF16To8Append(data, unprinted, data.size() - unprinted, next_token);
          json.array().value(next_token).close();
        }
        json.close().finish(true);
        return false;
      }

      spellchecker->Suggestions(tokens, corrections, suggestions - 1);

      for (unsigned i = 0; i < tokens.size(); i++)
        if (corrections[i].type != SpellcheckerCorrection::NONE) {
          if (unprinted < tokens[i]->first) {
            next_token.clear();
            UTF::UTF16To8Append(data, unprinted, tokens[i]->first - unprinted, next_token);
            json.array().value(next_token).close();
          }

          json.array();
          next_token.clear(); UTF::UTF16To8Append(tokens[i]->str, next_token); json.value(next_token);
          next_token.clear(); UTF::UTF16To8Append(corrections[i].correction, next_token); json.value(next_token);
          for (auto&& alternative : corrections[i].alternatives) {
            next_token.clear(); UTF::UTF16To8Append(alternative, next_token); json.value(next_token);
          }
          json.close();
          unprinted = tokens[i]->first + tokens[i]->length;
        }

      return true;
    }

   private:
    u16string data;
    unsigned unprinted;
    unique_ptr<InputFormat> input_format;
    unique_ptr<SpellcheckerI> spellchecker;
    unsigned suggestions;
    vector<TokenP> tokens;
    vector<SpellcheckerCorrection> corrections;
    string next_token;
  };
  return req.respond(ufal::microrestd::json_builder::mime, new Generator(model, *data, input_format.release(), suggestions));
}

bool KorektorService::HandleCorrect(ufal::microrestd::rest_request& req) {
  string error;
  auto data = GetData(req, error); if (!data) return req.respond_error(error);
  auto model = LoadSpellchecker(GetModelId(req), error); if (!model) return req.respond_error(error);
  auto input_format = InputFormat::NewInputFormat(GetInputFormat(req), model->spellchecker->Lexicon());
  if (!input_format) return req.respond_error("Unsupported input format.\n");

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

// Weblicht service
bool KorektorService::HandleWeblicht(ufal::microrestd::rest_request& req) {
  string language, text, error;

  // Parse the input TCF
  if (!ParseTCF(req, language, text, error)) return req.respond_error(error);

  // Get the required model
  auto id = GetModelId(req);
  if (id.empty()) return req.respond_error("The required parameter 'model' was not provided.\n");
  auto model = LoadSpellchecker(id, error);
  if (!model) return req.respond_error(error);

  // Check that the languages are consistent
  if (!language.empty() && !model->language.empty() && language != model->language)
    return error.assign("The language of the TCF '").append(language).append("' is not consistent with the model language '").append(model->language).append("'.\n"), req.respond_error(error);

  // TODO
  return req.respond_error("Not implemented");
}

const char* KorektorService::tcf_mime = "text/tcf+xml";

bool KorektorService::ParseTCF(const ufal::microrestd::rest_request& req, string& language, string& text, string& error) {
  using namespace ufal::microrestd::pugi;

  // Allow either empty Content-Type, ore one containing text/tcf+xml.
  if (!req.content_type.empty() && req.content_type.find(tcf_mime) == string::npos)
    return error.assign("Unsupported content type'").append(req.content_type).append("', expected text/tcf+xml.\n"), false;

  xml_document tcf;
  auto result = tcf.load_buffer_inplace((void*) req.body.c_str(), req.body.size());
  if (result.status != status_ok) return error.assign("Cannot parse the TCF: ").append(result.description()).append(".\n"), false;

  // Find required <text> element.
  auto tcf_text_element = tcf.child("D-Spin").child("TextCorpus").child("text");
  if (tcf_text_element.empty()) return error.assign("Cannot find <text> element in the given TCF.\n"), false;
  text.assign(tcf_text_element.text().get());

  // Find required lang attribute.
  auto lang_attribute = tcf.child("D-Spin").child("TextCorpus").attribute("lang");
  if (lang_attribute.empty()) return error.assign("Cannot find 'lang' attribute of the <TextCorpus> element.\n"), false;
  language.assign(lang_attribute.value());

  return true;
}

} // namespace korektor
} // namespace ufal
