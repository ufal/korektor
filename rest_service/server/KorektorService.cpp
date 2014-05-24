// Copyright 2013 by Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.

#include "StdAfx.h"
#include "Configuration.hpp"
#include "Spellchecker.hpp"
#include "Tokenizer.hpp"

#include "utf8.hpp"
#include "unicode.hpp"
#include "uninorms.hpp"
using namespace ufal::unilib;

#include "KorektorService.h"
#include "ResponseGenerator.h"

namespace ngramchecker {

static const string strip_diacritics = "strip_diacritics";

// Custom ResponseGenerator using JsonBuilder as data storage.
// Pointer to input data is used and additional logic for generate
// is implemented.
class KorektorResponseGenerator : public ResponseGenerator {
 public:
  KorektorResponseGenerator(const string& model, const char* data);

  virtual bool next() = 0;

  virtual StringPiece current() override;
  virtual void consume(size_t length) override;
  virtual bool generate() override;

 protected:
  const char* data;
  JsonBuilder result;
};

KorektorResponseGenerator::KorektorResponseGenerator(const string& model, const char* data) : data(data) {
  result.object().key("model").value(model).key("result");
}

StringPiece KorektorResponseGenerator::current() {
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

// Spellchecker implementation using Korektor
class KorektorService::KorektorProvider : public KorektorService::SpellcheckerProvider {
 public:
  KorektorProvider(const string& file) : configuration(new Configuration(string(), file)) {}

  class KorektorSpellchecker : public Spellchecker {
   public:
    KorektorSpellchecker(Configuration* configuration) : spellchecker(configuration) {}
    virtual void suggestions(const string& str, unsigned num, vector<pair<string, vector<string>>>& suggestions) override {
      spellchecker.GetSuggestions(str, num, suggestions);
    }
   private:
    ngramchecker::Spellchecker spellchecker;
  };

  virtual Spellchecker* new_spellchecker() const override {
    return new KorektorSpellchecker(configuration.get());
  }
 private:
  unique_ptr<Configuration> configuration;
};

// Spellchecker implementation stripping diacritics
class KorektorService::StripDiacriticsProvider : public KorektorService::SpellcheckerProvider {
 public:
  class StripDiacriticsSpellchecker : public Spellchecker {
   public:
    virtual void suggestions(const string& str, unsigned /*num*/, vector<pair<string, vector<string>>>& suggestions) override {
      suggestions.clear();

      u16string text = MyUtils::utf8_to_utf16(str);
      vector<vector<TokenP>> sentences = tokenizer.Tokenize(text);
      unsigned text_index = 0;
      for (auto&& sentence : sentences)
        for (auto&& token : sentence) {
          // Strip diacritics from token
          utf8::decode(token->str_utf8, token_utf32);
          uninorms::nfd(token_utf32);

          stripped_utf32.clear();
          for (auto&& chr : token_utf32)
            if (unicode::category(chr) & ~unicode::Mn)
              stripped_utf32 += chr;
          if (stripped_utf32.size() == token_utf32.size()) continue;

          uninorms::nfc(stripped_utf32);

          if (text_index < token->first) suggestions.emplace_back(MyUtils::utf16_to_utf8(text.substr(text_index, token->first - text_index)), vector<string>());
          utf8::encode(stripped_utf32, stripped);
          suggestions.emplace_back(token->str_utf8, vector<string>{stripped});

          text_index = token->first + token->length;
        }
      if (text_index < text.size()) suggestions.emplace_back(MyUtils::utf16_to_utf8(text.substr(text_index)), vector<string>());
    }
   private:
    Tokenizer tokenizer;
    string stripped;
    u32string token_utf32, stripped_utf32;
  };

  virtual Spellchecker* new_spellchecker() const override {
    return new StripDiacriticsSpellchecker();
  }
};

// Init the Korektor service -- load the spellcheckers
void KorektorService::init(const vector<SpellcheckerDescription>& spellchecker_descriptions) {
  // Load spellcheckers
  spellcheckers.clear();
  json_models.clear().object().key("models").array();
  for (auto& spellchecker_description : spellchecker_descriptions) {
    spellcheckers.emplace(spellchecker_description.id, unique_ptr<SpellcheckerProvider>(new KorektorProvider(spellchecker_description.file)));
    json_models.value(spellchecker_description.id);
  }
  spellcheckers.emplace(strip_diacritics, unique_ptr<SpellcheckerProvider>(new StripDiacriticsProvider()));
  json_models.value(strip_diacritics);

  default_model = spellchecker_descriptions.empty() ? strip_diacritics : spellchecker_descriptions.front().id;
  json_models.close().key("default_model").value(default_model);
}

// Handle a request
bool KorektorService::handle(RestRequest& req) {
  if (req.url == "/models") return req.respond_json(json_models);
  if (req.url == "/correct") return handle_correct(req);
  if (req.url == "/suggestions") return handle_suggestions(req);
  return req.respond_not_found();
}

bool KorektorService::handle_suggestions(RestRequest& req) {
  JsonBuilder error;
  auto data = get_data(req, error); if (!data) return req.respond_json(error);
  string model;
  auto provider = get_provider(req, model, error); if (!provider) return req.respond_json(error);
  auto suggestions = get_suggestions(req, error); if (!suggestions) return req.respond_json(error);

   class generator : public KorektorResponseGenerator {
   public:
    generator(const string& model, const char* data, unsigned num, Spellchecker* spellchecker)
        : KorektorResponseGenerator(model, data), num(num), spellchecker(spellchecker) { result.array(); }

    bool next() {
      StringPiece line;
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
    unique_ptr<Spellchecker> spellchecker;
    vector<pair<string, vector<string>>> suggestions;
  };
  return req.respond_json(new generator(model, data, suggestions, provider->new_spellchecker()));
}

bool KorektorService::handle_correct(RestRequest& req) {
  JsonBuilder error;
  auto data = get_data(req, error); if (!data) return req.respond_json(error);
  string model;
  auto provider = get_provider(req, model, error); if (!provider) return req.respond_json(error);

   class generator : public KorektorResponseGenerator {
   public:
    generator(const string& model, const char* data, Spellchecker* spellchecker)
        : KorektorResponseGenerator(model, data), spellchecker(spellchecker) { result.value_open(); }

    bool next() {
      StringPiece line;
      if (!get_line(data, line)) return result.value_close(), false;

      spellchecker->suggestions(string(line.str, line.len), 1, suggestions);
      for (auto&& suggestion : suggestions)
        result.value_append(suggestion.second.empty() ? suggestion.first : suggestion.second.front());

      return true;
    }

   private:
    unique_ptr<Spellchecker> spellchecker;
    vector<pair<string, vector<string>>> suggestions;
  };
  return req.respond_json(new generator(model, data, provider->new_spellchecker()));
}

const char* KorektorService::get_data(RestRequest& req, JsonBuilder& error) {
  auto data_it = req.params.find("data");
  if (data_it == req.params.end()) return error.clear().object().key("error").value("Required argument 'data' is missing."), nullptr;

  return data_it->second.c_str();
}

const KorektorService::SpellcheckerProvider* KorektorService::get_provider(RestRequest& req, string& model, JsonBuilder& error) {
  auto data_it = req.params.find("model");
  model = data_it == req.params.end() ? default_model : data_it->second;
  auto spellchecker_it = spellcheckers.find(model);
  if (spellchecker_it == spellcheckers.end()) return error.clear().object().key("error").value("Specified model '" + model + "' does not exist."), nullptr;
  return spellchecker_it->second.get();
}

unsigned KorektorService::get_suggestions(RestRequest& req, JsonBuilder& error) {
  auto data_it = req.params.find("suggestions");
  if (data_it == req.params.end()) return 5;

  int suggestions = atoi(data_it->second.c_str());
  if (suggestions <= 0) return error.clear().object().key("error").value("Specified number of suggestions '" + data_it->second + "' is not a positive integer"), 0;

  return suggestions;
}

bool KorektorService::get_line(const char*& data, StringPiece& line) {
  line.str = data;
  while (*data && *data != '\r' && *data != '\n') data++;
  line.len = data - line.str;

  // If end of data was reached
  if (!*data) return line.len;

  // Otherwise, include the line ending
  if (*data == '\r') {
    data++;
    if (*data == '\n') data++;
  } else if (*data == '\n') {
    data++;
    if (*data == '\r') data++;
  }
  line.len = data - line.str;
  return true;
}

} // namespace ngramchecker
