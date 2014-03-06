// Copyright 2013 by Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.

#ifndef JSON_BUILDER_H
#define JSON_BUILDER_H

#include "StdAfx.h"

#include "StringPiece.h"

namespace ngramchecker {

// Declarations
class JsonBuilder {
 public:
  // Clear
  inline JsonBuilder& clear();

  // Encode
  inline JsonBuilder& object();
  inline JsonBuilder& array();
  inline JsonBuilder& close();
  inline JsonBuilder& key(StringPiece str);
  inline JsonBuilder& value(StringPiece str);
  inline void close_all(); // Close all open objects and arrays

  // Return current json
  inline StringPiece current();

  // Operator which calls close_all() and current();
  inline operator StringPiece();

  // Remove current json prefix; for ResponseGenerator
  inline void discard_prefix(size_t length);

 private:
  inline void start_element(bool key);
  inline void encode(StringPiece str);

  vector<char> json;
  vector<char> stack;
  bool comma_needed = false;
};


// Definitions
JsonBuilder& JsonBuilder::clear() {
  json.clear();
  stack.clear();
  comma_needed = false;
  return *this;
}

JsonBuilder& JsonBuilder::object() {
  start_element(false);
  json.push_back('{');
  json.push_back('\n');
  stack.push_back('}');
  comma_needed = false;
  return *this;
}

JsonBuilder& JsonBuilder::array() {
  start_element(false);
  json.push_back('[');
  json.push_back('\n');
  stack.push_back(']');
  comma_needed = false;
  return *this;
}

JsonBuilder& JsonBuilder::close() {
  if (!stack.empty()) {
    json.insert(json.end(), stack.size() - 1, ' ');
    json.push_back(stack.back());
    json.push_back('\n');
    stack.pop_back();
    comma_needed = true;
  }
  return *this;
}

JsonBuilder& JsonBuilder::key(StringPiece str) {
  start_element(true);
  json.push_back('"');
  encode(str);
  json.push_back('"');
  json.push_back(':');
  json.push_back(' ');
  comma_needed = false;
  return *this;
}

JsonBuilder& JsonBuilder::value(StringPiece str) {
  start_element(false);
  json.push_back('"');
  encode(str);
  json.push_back('"');
  json.push_back('\n');
  comma_needed = true;
  return *this;
}

void JsonBuilder::close_all() {
  while (!stack.empty()) close();
}

StringPiece JsonBuilder::current() {
  return StringPiece(json.data(), json.size());
}

JsonBuilder::operator StringPiece() {
  close_all();
  return current();
}

void JsonBuilder::discard_prefix(size_t length) {
  if (length >= json.size()) json.clear();
  else json.erase(json.begin(), json.begin() + length);
}

void JsonBuilder::start_element(bool key) {
  if (stack.empty() || stack.back() != '}' || key) {
    if (!stack.empty()) json.insert(json.end(), stack.size() - (comma_needed ? 1 : 0), ' ');
    if (comma_needed) json.push_back(',');
  }
}

void JsonBuilder::encode(StringPiece str) {
  for (; str.len; str.str++, str.len--)
    switch (*str.str) {
      case '"': json.push_back('\\'); json.push_back('\"'); break;
      case '\\': json.push_back('\\'); json.push_back('\\'); break;
      case '\b': json.push_back('\\'); json.push_back('b'); break;
      case '\f': json.push_back('\\'); json.push_back('f'); break;
      case '\n': json.push_back('\\'); json.push_back('n'); break;
      case '\r': json.push_back('\\'); json.push_back('r'); break;
      case '\t': json.push_back('\\'); json.push_back('t'); break;
      default:
        if (((unsigned char)*str.str) < 32) {
          json.push_back('u'); json.push_back('0'); json.push_back('0'); json.push_back('0' + (*str.str >> 4)); json.push_back("0123456789ABCDEF"[*str.str & 0xF]);
        } else {
          json.push_back(*str.str);
        }
    }
}

} // namespace ngramchecker

#endif
