/** Implements an improved JSON5 weak-syntax reader and writer.
 * @class
 */
var wJSON = {
  /** Parses a data structure from a JSON weak-syntax string.
   * @param {string} value The value given as a string, using weak [JSON](http://json.org) syntax, derived from [JSON5](https://spec.json5.org).
   * @return {value} The parsed data-structure.
   */
  parse: function(value) {
    const Reader = {
      string: "" + value,
      index: 0,
      read: function() {
        let value = this.read_value();
        this.next_space();
        if (this.index < this.string.length) {
          return {
            value: value,
            trailer: this.string.substr(this.index)
          };
        }
        return value;
      },
      read_value: function() {
        this.next_space();
        switch (this.string[this.index]) {
          case '{':
            return this.read_tuple_value();
          case '[':
            return this.read_list_value();
          default:
            return this.string2value(this.read_word(false));
        }
      },
      read_tuple_value: function() {
        let value = {};
        this.index++;
        for (let index0 = -1; index0 != this.index;) {
          index0 = this.index;
          this.next_space();
          if (this.index >= this.string.length) {
            return value;
          }
          if (this.string[this.index] == '}') {
            this.index++;
            return value;
          }
          let name = this.read_word();
          if (name == '') {
            return value;
          }
          this.next_space();
          let item = true;
          if (this.read_punctuation([':', '='])) {
            item = this.read_value();
          }
          value[name] = item;
          this.next_space();
          this.read_punctuation([',', ';']);
        }
      },
      read_list_value: function() {
        let value = [];
        this.index++;
        for (let index0 = -1; index0 != this.index;) {
          index0 = this.index;
          this.next_space();
          if (this.index >= this.string.length) {
            return value;
          }
          if (this.string[this.index] == ']') {
            this.index++;
            return value;
          }
          value.push(this.read_value());
          this.next_space();
          this.read_punctuation([',', ';']);
        }
      },
      read_punctuation: function(symbols) {
        let found = false;
        while (this.index < this.string.length) {
          this.next_space();
          if (!symbols.includes(this.string[this.index])) {
            break;
          }
          found = true;
          this.index++;
        }
        return found;
      },
      read_word: function(line = false) {
        return this.string[this.index] == '"' || this.string[this.index] == '\'' ? this.read_quoted_word(this.string[this.index]) : this.read_nospace_word(line);
      },
      read_quoted_word: function(quote) {
        let word = "";
        for (this.index++; this.index < this.string.length && this.string[this.index] != quote; this.index++) {
          if ((this.string[this.index] == '\\') && (this.index < this.string.length - 1)) {
            this.index++;
            switch (this.string[this.index]) {
              case '\'':
              case '"':
              case '\\':
              case '/':
                word += this.string[this.index];
                break;
              case 'n':
                word += "\n";
                break;
              case 'b':
                word += "\b";
                break;
              case 'r':
                word += "\r";
                break;
              case 't':
                word += "\t";
                break;
              case 'f':
                word += "\f";
                break;
              default:
                word += "\\";
                word += this.string[this.index];
            }
          } else {
            word += this.string[this.index];
          }
        }
        if (this.index < this.string.length) {
          this.index++;
        }
        return word;
      },
      read_nospace_word: function(line = false) {
        let i0;
        for (i0 = this.index; this.index < this.string.length && (line ? this.no_endofline(this.string[this.index]) : this.no_space(this.string[this.index])); this.index++) {}
        return this.string.substr(i0, this.index - i0).trim();
      },
      next_space: function() {
        for (; this.index < this.string.length && this.isspace(this.string[this.index]); this.index++) {}
        if ((this.index < this.string.length) &&
          ((this.string[this.index] == '#') ||
            ((this.string[this.index] == '/') && ((this.index == this.string.length - 1) || (this.string[this.index + 1] == '/'))))) {
          for (; this.index < this.string.length && this.string[this.index] != '\n'; this.index++) {}
          this.next_space();
        }
      },
      string2value: function(string) {
        if (new RegExp("^(true|false)$").test(string)) {
          return string == "true";
        } else if (new RegExp("^[-+]?[0-9]+$").test(string)) {
          return parseInt(string);
        } else if (new RegExp("^[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?$").test(string)) {
          return parseFloat(string);
        } else {
          return string;
        }
      },
      no_space: function(c) {
        if (c == ":")
          return this.index < this.string.length - 1 && new RegExp("[^\\s,;:={}[\\]'\"]").test(this.string[this.index + 1]);
        else
          return new RegExp("[^\\s,;:=}\\]'\"]").test(c);
      },
      no_endofline: function(c) {
        return new RegExp("[^\\n,;:=}\\]]").test(c);
      },
      isspace: function(c) {
        return new RegExp("\\s").test(c);
      }
    };
    return Reader.read();
  },

  /** Converts a data structure the a light weak JSON syntax with a minimal number of quotes.
   * @param {value} value The parsed data-structure.
   * @param {bool} [pretty=false] 
   *  - If true properly format in 2D.
   *  - If "html" returns a colored HTML 2D string to vizualize the result.
   *  - If "latex" returns a latex header and a latex body to copy paste, as [shown here](./wjson_test.js_latex_stringify.pdf) [(source)](./wjson_test.js_latex_stringify.tex).
   *  - If "minified" returns a string with a minimal number of chars.
   *  - If false returns a readable one-line raw format.
   * @return {string} A 2D formated string view of the value.
   */
  stringify: function(value, pretty = false) {
    let Writer = {
      write: function(value) {
        const css_style = "<style>.wjson {background-color: lightgray; font-weight:bold; display: inline-block; padding:10px; } .wjson .name { font-weight: normal; color: red;} .wjson .value { font-weight: normal; color: green;}</style>";
        const latex_header =
          "\\documentclass{article}\n" +
          "\\usepackage{listings}\n" +
          "\\usepackage{xcolor}\n" +
          "" +
          "\\lstdefinelanguage{json}{" +
          "basicstyle=\\normalfont\\ttfamily," +
          " numbers=left," +
          " numberstyle=\\scriptsize," +
          " breaklines=true," +
          " frame=lines," +
          " backgroundcolor=\\color{gray!10}," +
          " showstringspaces=false," +
          " string=[db]{\"}," +
          " stringstyle=\\color{green!50!black}," +
          " morestring=[s][\\color{black}]{\\ \\ \"}{\":}," +
          " keywordstyle=\\color{blue}," +
          " keywords={true,false,null}," +
          " literate=" +
          " *{0}{{{\\color{red}0}}}{1}" +
          " {1}{{{\\color{red}1}}}{1}" +
          " {2}{{{\\color{red}2}}}{1}" +
          " {3}{{{\\color{red}3}}}{1}" +
          " {4}{{{\\color{red}4}}}{1}" +
          " {5}{{{\\color{red}5}}}{1}" +
          " {6}{{{\\color{red}6}}}{1}" +
          " {7}{{{\\color{red}7}}}{1}" +
          " {8}{{{\\color{red}8}}}{1}" +
          " {9}{{{\\color{red}9}}}{1}" +
          " {.}{{{\\color{red}.}}}{1}" +
          " {:}{{{\\color{gray}{:}}}}{1}" +
          " {,}{{{\\color{gray}{,}}}}{1}" +
          " {\\{}{{{\\color{gray}{\\{}}}}{1}" +
          " {\\}}{{{\\color{gray}{\\}}}}}{1}" +
          " {[}{{{\\color{gray}{[}}}}{1}" +
          " {]}{{{\\color{gray}{]}}}}{1}," +
          "}\n\n";

        return (pretty == "html" ? css_style + "<div class='wjson'>" : pretty == "latex" ? latex_header + "\n\\begin{document}\n\n\\begin{lstlisting}[language=json]\n" : "") + this.write_value(value) + (pretty == true ? "\n" : pretty == "html" ? "</div>" : pretty == "latex" ? "\n\\end{lstlisting}\n\n\\end{document}\n" : "");
      },
      write_value: function(value) {
        if (!(value instanceof Object)) {
          return this.write_word(value);
        } else if (value instanceof Array) {
          this.itab++;
          let strings = [];
          for (let i = 0; i < value.length; i++) {
            strings.push(this.write_value(value[i]));
          }
          let result = this.write_strings("[", "]", strings);
          return result;
        } else {
          this.itab++;
          let strings = [];
          for (let label in value) {
            let v = this.write_value(value[label]);
            strings.push(this.write_word(label, "name") + (pretty == "minified" && v == "true" ? "" : ":" + (pretty != "minified" || !new RegExp("[\"{}[\\]]").test(v[0]) ? " " : "") + v));
          }
          return this.write_strings("{", "}", strings);
        }
      },
      write_word: function(value, what = "value") {
        let string = String(value);
        let quoted = string == "" || new RegExp("[\\s,;:={}[\\]]").test(string);
        if (pretty == "html")
          string = string.replaceAll("\n", "<br/>\n").replaceAll("\t", "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
        if (quoted)
          string = string.replace(new RegExp("([\"\\\\])", "g"), "\\$1");
        return (quoted ? "\"" : "") + (pretty == "html" ? "<span class='" + what + "'>" + string + "</span>" : string) + (quoted ? "\"" : "");
      },
      write_strings: function(start, stop, strings) {
        let string = start,
          length = 2,
          old_pretty = pretty;
        // Computes the total lengths to sdecide if write inline
        {
          for (let i in strings) {
            let s = (pretty == "html") ? strings[i].replaceAll(new RegExp("(<[^>]*>|&[^;]*;)", "g"), "") : strings[i];
            length += 1 + s.length;
          }
        }
        if (pretty != "minified" && length < 120) {
          pretty = false;
        }
        for (let i in strings) {
          string += (pretty != "minified" || (0 < i && string[string.length - 1] != '}' && string[string.length - 1] != ']') ? this.write_line() : "") + strings[i];
        }
        this.itab--;
        string = string + (pretty != "minified" ? this.write_line() : "") + stop;
        pretty = old_pretty;
        return string;
      },
      write_line: function() {
        if (pretty == true || pretty == "latex") {
          let string = "\n";
          for (let i = 0; i < this.itab; i++) {
            string += "  ";
          }
          return string;
        } else if (pretty == "html") {
          let string = "<br/>";
          for (let i = 0; i < this.itab; i++) {
            string += "&nbsp;&nbsp;";
          }
          return string;
        } else {
          return " ";
        }
      },
      itab: 0
    };
    return Writer.write(value);
  }
};

module.exports = wJSON;
