// Implements the weak syntax JSON reader

#include "WJSONReader.hpp"
// #include "regex.hpp" no more used

namespace wjson {
  void WJSONReader::read(String string, Value& value)
  {
    chars = string.c_str(), index = 0, length = string.length();
    read_value(value);
    next_space();
    if(index < length) {
      value["WJSONReader_trailer"] = std::string(chars + index);
    }
  }
  // Reads a value
  void WJSONReader::read_value(Value& value)
  {
    next_space();
    if(index < length) {
      switch(chars[index]) {
      case '{':
        value.type = Value::Type::record;
        read_tuple_value(value);
        break;
      case '[':
        value.type = Value::Type::array;
        read_list_value(value);
        break;
      default:
        std::string word;
        read_word(word);
        value = word;
        break;
      }
    }
  }
  // Reads a JSON object, or record enclosed between brackets { }
  void WJSONReader::read_tuple_value(Value& value)
  {
    index++;
    for(unsigned int index0 = -1; index0 != index;) {
      index0 = index;
      next_space();
      if(index >= length) {
        return;
      }
      if(chars[index] == '}') {
        index++;
        return;
      }
      std::string name;
      read_word(name);
      if(name == "") {
        return;
      }
      next_space();
      if(read_punctuation(":=")) {
        read_value(value[name]);
      } else {
        value[name] = true;
      }
      next_space();
      read_punctuation(",;");
    }
  }
  // Reads a JSON array, or list enclosed between square brackets [ ]
  void WJSONReader::read_list_value(Value& value)
  {
    index++;
    for(unsigned int index0 = -1; index0 != index;) {
      index0 = index;
      next_space();
      if(index >= length) {
        return;
      }
      if(chars[index] == ']') {
        index++;
        return;
      }
      read_value(value[value.length()]);
      next_space();
      read_punctuation(",;");
    }
  }
  // Reads a word
  void WJSONReader::read_word(std::string& word)
  {
    if(chars[index] == '"' || chars[index] == '\'') {
      read_quoted_word(chars[index], word);
    } else {
      read_nospace_word(word);
    }
  }
  // Reads a word between quote
  void WJSONReader::read_quoted_word(char quote, std::string& word)
  {
    for(index++; index < length && chars[index] != quote; index++) {
      if(chars[index] == '\\' && index < length - 1 && chars[index + 1] == quote) {
        index++;
        word += chars[index];
      } else {
        word += chars[index];
      }
    }
    if(index < length) {
      index++;
    }
  }
  // Reads a word until next space
  void WJSONReader::read_nospace_word(std::string& word)
  {
    for(; index < length && no_space(index); index++) {
      word += chars[index];
    }
  }
  // Escapes a conjunction of punctuation chars
  bool WJSONReader::read_punctuation(String symbols)
  {
    bool found = false;
    while(index < length) {
      next_space();
      if(symbols.find(chars[index]) == std::string::npos) {
        break;
      }
      found = true;
      index++;
    }
    return found;
  }
  // Returns true if the char is not a space or a punctuation
  bool WJSONReader::no_space(unsigned int index)
  {
    char c = chars[index];
    switch(c) {
    case ':':
      return index < length - 1 && no_space(index + 1);
    case '"':
    case ',':
    case ';':
    case '=':
    case '{':
    case '}':
    case '[':
    case ']':
      return false;
    default:
      return !isspace(c);
    }
  }
  // Escapes until next non space char
  void WJSONReader::next_space()
  {
    // Escapes spaces
    for(; index < length && isspace(chars[index]); index++) {}
    // Escapes comments
    if((index < length) &&
       ((chars[index] == '#') ||
        ((chars[index] == '/') && ((index == length - 1) || (chars[index + 1] == '/')))))
    {
      for(; index < length && chars[index] != '\n'; index++) {}
      next_space();
    }
  }
}
