#ifndef __wjson_WJSONWriter__
#define __wjson_WJSONWriter__

#include "std.hpp"
#include "Value.hpp"
#include <string.h>
#include "file.hpp" // @debug

namespace wjson {
  /** Implements a weak JSON writer.
   * @class WJSONWriter
   */
  class WJSONWriter {
    unsigned int itab;
    bool pretty, strict, fast;
    // Reimplements a very basic string buffer to increase performances, with a specific sequences of actions
    // - Buffer buffer;                // 1. Buffer initialization
    // - buffer.putc(c);               // 2. A lot of appends ../..
    // - std::string = buffer.gets();  // 3. Buffer is closed and converted to a string
    class Buffer {
      // Maximal line width for pretty writing of lists
      friend class WJSONWriter;
      unsigned int buffer_length = 128, size = 0;
      char *buffer;
public:
      Buffer() : buffer(new char[buffer_length]) {}
      ~Buffer() {
        delete[] buffer;
      }
      // Appends a char to the buffer
      void putc(char c)
      {
        check_size();
        buffer[size++] = c;
      }
      // Closes the buffer and returns its value
      const char *gets()
      {
        putc('\0');
        return buffer;
      }
      void check_size()
      {
        const static unsigned int increase_scale = 2, maximal_length = 100 * 1000000;
        if(size == buffer_length - 1) {
          const char *b = buffer;
          unsigned int l = buffer_length;
          buffer_length *= increase_scale;
          if(buffer_length > maximal_length) {
            aidesys::save("wjson::WJSONWriter::Buffer::dump.txt", gets());
          }
          aidesys::alert(buffer_length > maximal_length, "illegal-state", "in wjson::WJSONWriter::Buffer:putc buffer overflows");
          buffer = new char[buffer_length];
          memcpy(buffer, b, l);
          delete[] b;
        }
      }
    };
public:

    /**
     * @function write
     * @memberof WJSONWriter
     * @description Returns a JSON structure as a weak JSON string.
     * @param {JSON} value The input JSON value to stringify.
     * @param {bool} [pretty=false] If true properly format in 2D, else returns a minimal raw format.
     * @param {bool} [strict=false] If true [strict JSON](https://www.json.org/json-en.html) syntax, else produces a [weak JSON](./wJSON.html#.parse) syntax.
     * @param {bool} [fast=false] If fast serializes the data as fast as possible (quoting all words without any lexical analysis).
     */
    std::string write(JSON value, bool pretty = false, bool strict = false, bool fast = false);
private:
    void write_value(JSON value, Buffer& string);
    void write_word(JSON value, bool name, Buffer& string);
    void write_line(Buffer& string);
public:
    static bool force_fast; // This is only used for performance tests.
  };
}
#endif
