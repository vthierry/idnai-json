// Implements the weak syntax JSON writer

#include "WJSONWriter.hpp"
#include "regex.hpp"

namespace wjson {
  std::string WJSONWriter::write(JSON value, bool pretty, bool strict, bool fast)
  {
    itab = 0, this->pretty = pretty, this->strict = strict, this->fast = fast || force_fast;
    Buffer string;
    write_value(value, string);
    if(pretty) {
      string.putc('\n');
    }
    return string.gets();
  }
  void WJSONWriter::write_value(JSON value, Buffer& string)
  {
    if(value.isRecord()) {
      itab++;
      if(value.isArray()) {
        unsigned int i0 = string.size;
        string.putc('[');
        for(unsigned int i = 0; i < value.length(); i++) {
          if(strict && i > 0) {
            string.putc(',');
          }
          write_line(string);
          write_value(value.at(i), string);
        }
        itab--;
        write_line(string);
        string.putc(']');
        // Prints short lists in one line
        if(pretty) {
          const static unsigned int maximal_width = 120;
          unsigned int i1 = string.size;
          if(i1 - i0 < maximal_width && std::string(string.buffer + i0 + 1, i1 - i0).find("[") == std::string::npos) {
            std::string line = aidesys::regexReplace(std::string(string.buffer + i0, i1 - i0), "\\s+", " ");
            string.size = i0;
            for(auto it = line.begin(); it != line.end(); it++) {
              string.putc(*it);
            }
          }
        }
      } else {
        unsigned int i0 = string.size;
        string.putc('{');
        bool next = false;
        for(auto it = value.getNames().begin(); it != value.getNames().end(); ++it, next = true) {
          JSON value_it = value.at(*it);
          if(value_it.isRecord() || !value_it.isEmpty()) {
            if(strict && next) {
              string.putc(',');
            }
            write_line(string);
            write_word(*it, true, string);
            if(strict || !value_it.get(false)) {
              string.putc(':');
              if(pretty || !strict) {
                string.putc(' ');
              }
              write_value(value_it, string);
            }
          }
        }
        itab--;
        write_line(string);
        string.putc('}');
        // Prints short records in one line
        if(pretty) {
          const static unsigned int maximal_width = 120;
          unsigned int i1 = string.size;
          if(i1 - i0 < maximal_width && std::string(string.buffer + i0 + 1, i1 - i0).find("{") == std::string::npos) {
            std::string line = aidesys::regexReplace(std::string(string.buffer + i0, i1 - i0), "\\s+", " ");
            string.size = i0;
            for(auto it = line.begin(); it != line.end(); it++) {
              string.putc(*it);
            }
          }
        }
      }
    } else {
      write_word(value, false, string);
    }
  }
  void WJSONWriter::write_word(JSON value, bool name, Buffer& string)
  {
    String word = value.get("");
    if(fast ||
       word == "" ||
       aidesys::regexMatch(word, "^.*[,;={}[\\]\"\\s].*$") ||
       aidesys::regexMatch(word, "^.*:$") ||
       (strict
        && (name ||
            !aidesys::regexMatch(word, "^(true|false|[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)$"))))
    {
      string.putc('"');
      for(auto it = word.begin(); it != word.end(); ++it) {
        if(*it == '"') {
          string.putc('\\');
        }
        string.putc(*it);
      }
      string.putc('"');
    } else {
      for(auto it = word.begin(); it != word.end(); ++it) {
        string.putc(*it);
      }
    }
  }
  void WJSONWriter::write_line(Buffer& string)
  {
    if(pretty) {
      string.putc('\n');
      for(unsigned int i = 0; i < itab; i++) {
        string.putc(' '), string.putc(' ');
      }
    } else if(!strict) {
      string.putc(' ');
    }
  }
  bool WJSONWriter::force_fast;
}
