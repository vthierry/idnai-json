#ifndef __wjson_WJSONReader__
#define __wjson_WJSONReader__

#include "std.hpp"
#include "Value.hpp"

namespace wjson {
  /** Implements a weak JSON reader.
   * @class WJSONReader
   */
  class WJSONReader {
private:
    const char *chars;
    unsigned int index, length;
public:

    /**
     * @function read
     * @memberof WJSONReader
     * @description Reads a string and returns the best parsed structure.
     * @param {string} string The input string to parse.
     * @param {Value} value The output value where parsed value are inserted.
     */
    void read(String string, Value& value);
private:
    void read_value(Value& value);
    void read_tuple_value(Value& value);
    void read_list_value(Value& value);
    void read_word(std::string& word);
    void read_quoted_word(char quote, std::string& word);
    void read_nospace_word(std::string& word);
    bool read_punctuation(String symbols);
    void next_space();
    bool no_space(unsigned int index);
  };
}
#endif
