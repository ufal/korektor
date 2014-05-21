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

  virtual Spellchecker* new_provider() const override {
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

  virtual Spellchecker* new_provider() const override {
    return new StripDiacriticsSpellchecker();
  }
};

// Init the Korektor service -- load the spellcheckers
void KorektorService::init(const vector<SpellcheckerDescription>& spellchecker_descriptions) {
  // Load spellcheckers
  spellcheckers.clear();
  json_spellcheckers.clear().object().key("models").array();
  for (auto& spellchecker_description : spellchecker_descriptions) {
    spellcheckers.emplace("/" + spellchecker_description.id, unique_ptr<KorektorProvider>(new KorektorProvider(spellchecker_description.file)));
    json_spellcheckers.value(spellchecker_description.id);
  }
  default_spellchecker = spellchecker_descriptions.empty() ? strip_diacritics : spellchecker_descriptions.front().id;
  json_spellcheckers.value(strip_diacritics).close().key("default_model").value(default_spellchecker);
}

// Handle a request using the specified URL/handler map
bool KorektorService::handle(RestRequest& req) {
  if (req.url == "/strip_diacritics") return handle_strip(req);

  auto spellchecker_it = spellcheckers.find(req.url);
  if (spellchecker_it == spellcheckers.end()) return req.respond_not_found();

  JsonBuilder error;
  auto data = get_data(req, error); if (!data) return req.respond_json(error);

  class generator : public KorektorResponseGenerator {
   public:
    generator(const string& model, const char* data, Spellchecker* spellchecker) : KorektorResponseGenerator(model, data), spellchecker(spellchecker) {}

    bool next() {
      StringPiece line;
      if (!get_line(data, line)) return false;

      spellchecker->suggestions(string(line.str, line.len), 5, suggestions);
      for (auto&& suggestion : suggestions) {
        result.array().value(suggestion.first);
        for (auto&& word : suggestion.second)
          result.value(word);
        result.close();
      }

      return true;
    }

   private:
    unique_ptr<Spellchecker> spellchecker;
    vector<pair<string, vector<string>>> suggestions;
  };
  return req.respond_json(new generator(req.url, data, spellchecker_it->second->new_provider()));
}

bool KorektorService::handle_strip(RestRequest& req) {
  JsonBuilder error, output;
  auto data = get_data(req, error); if (!data) return req.respond_json(error);

  class generator : public KorektorResponseGenerator {
   public:
    generator(const string& model, const char* data) : KorektorResponseGenerator(model, data) {}

    bool next() {
      StringPiece line;
      if (!get_line(data, line))
        return false;

      u16string text = MyUtils::utf8_to_utf16(string(line.str, line.len));
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

          if (text_index < token->first) result.array().value(MyUtils::utf16_to_utf8(text.substr(text_index, token->first - text_index))).close();
          utf8::encode(stripped_utf32, stripped);
          result.array().value(token->str_utf8).value(stripped).close();

          text_index = token->first + token->length;
        }
      if (text_index < text.size()) result.array().value(MyUtils::utf16_to_utf8(text.substr(text_index))).close();

      return true;
    }
   private:
    Tokenizer tokenizer;
    string stripped;
    u32string token_utf32, stripped_utf32;
  };
  return req.respond_json(new generator(strip_diacritics, data));
}

const char* KorektorService::get_data(RestRequest& req, JsonBuilder& error) {
  auto data_it = req.params.find("data");
  if (data_it == req.params.end()) return error.clear().object().key("error").value("Required argument 'data' is missing."), nullptr;

  return data_it->second.c_str();
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
