## Presentation: A cool easy to write ``weak JSON´´

The [JSON](https://json.org) syntax is now an universal way of representing data structures.

We want a person who is unfamiliar with computer programming to be able to easily specify a data 
structure. All that is required is to understand the notion of "sequence" and the notion of "fields",
as explained now. In other words, JSON for Humans, as stated by the [JSON5](https://json5.org) initiative.

The present JSON for Humans specification and implementation is called `wJSON` for `weak JSON` 

### Understanding with an example

Let us consider this example:
```
 // A wjson data structure example
 {
   first-name: Jean-Pierre
   last-name: Pierrejean
   age: 107
   address: "314 Pi road, Quadrature city"
   friends: [ him, her, "somebody else" ]
   imaginative // (which a boolean attribute, equal to true)
   presentation: "
    Under the moonlight
    Write a word in white
    Please borrow my pen
    We are all nice men"
 }
```
This structured data is a kind of business card type, with field names (ex: `last-name` or` age`), 
textual or numeric values, a list of values, and a textual value on several lines.

We read that a guy called `Pierrejean, Jean-Pierre` who is very old and provides his address,
has a list of three friends, and is a (very poor) writer, while he is imaginative.

### Introducing the notion of weak syntax

- A structured data (we speak of named t-uple or [record](https://en.wikipedia.org/wiki/Record_(computer_science)), which is called an Object in the [JSON vocabulary](https://www.json.org/json-en.html) or when interfacing with a [JavaScript Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object)) is of the form `{name: value ...}` allowing us to define a data by a set of "fields", i.e., of named parameters, this is also called a "t-uple", using braces.
- A list of item is of the form `[value ...]` allowing us to define a sequence of data, using brackets.
- Elementary data are character strings, or numeric values, or boolean value (i.e., `false` or `true`).
- __Caution__: The most common error is to write `name:value` instead of `name: value`, the former is a compound work, the later a name/value pair.

Nothing more. This is enough to define all usual structured data.

- All this can be nested, allowing to build a hierarchical structured data.
- The fact that the `imaginative` field has no value simply means that it has the value `true`.
- Strings with spaces or on several lines are enclosed in quotes `" `, other strings can be written without quote.

As a consequence, by construction, any string maps onto a well-formed (eventually absurd) value.

### Interest of the approach

The weak JSON syntax for "weak JSON" compared to [strict JSON syntax] (https://www.w3schools.com/js/js_json_intro.asp) allows you to define a data structure without worrying too much about syntax details.

We want people without a technical background in computer science, but understanding the principles of 
information encoding, to be able to easily enter specifications such as structured data or instruction 
sequences.

It turns out, considering years of interaction with colleagues and end users,
that the data representation proposed by the JSON syntax is understood very well by people with 
only some basic computer science skills, remains very readable and easy to write.
But, with a "but". The "but" is that we really waste time because of forgotten or excess meta-characters, 
while this in no way harms the syntactic analysis. 
We therefore have to develop a mechanism for reading and writing in a syntax "light" which in use really 
makes things easier.

The risk is of course that some errors (for example a forgotten brace) lead to an erroneous data 
structure, but it is easy to verify this. For example, the output text can be put in a strict JSON 
syntax indented and human readable to verify that the input was well formed. Or, the text can be 
reformatted, indented, to show the structure clearly in order to control it.

<a name="semantic"></a>
### Semantic differences with standard JSON

- The main semantic difference is that the name/value pairs order matters, i.e., the insertion order of record keys is preserved by default, or it can be sorted in any application related order.

- A JSON array `[a b ...]` is equivalent and equal to a record `{ 0: a 1: b ...}` indexed by consecutive non negative integer.
  - In other words, record with _all_ name of pattern `(0|[1-9][0-9]*)` are interpreted as arrays.
  - Note: an positive integer is not expected to start with "0" (i.e., "1" is an integer, "01" is not).

- Each literal (i.e. atomic) value casts from and onto string, 
  - The string value "true" or "false" is equivalent and equal to the boolean value true.
  - Any string representation (e.g., "0.31416e1") of a numeric integral of floating point value is equivalent and equal to the corresponding value.

- The `empty` value corresponds to undefined value, but is neither input no output,
  - On input "empty" corresponds to the string "empty" not the empty value : Empty value are simply omitted.
  - On output "empty" value is not written.
  - As a literal, the empty value casts to the empty string `""`, the boolean `false` value, the integral number `0`, or the floating point number `NAN`.

- With this semantic, two data structures are equal for the modified semantic:
  - Two literal values are equal if and only the literal string value are equal.
  - Two data structures are equal if and only each item are inserted in the same order and equal.

### Complete description

- In a nutshell, the input syntax accepts (i) implicit quote <tt>"</tt> when string reduces to one word, (ii) optional use of comma <tt>,</tt>, (iii) string on several lines, (iv) considering <tt>true</tt> as implicit value, (v) appending as a raw string trailer chars if any, i.e., if the parsing ends with reminding chars.

- Any strict syntax JSON is also parsed by the weak syntax parser, indeed.

- The weak-syntax, with respect to the strict [JSON-syntax](https://www.w3schools.com/js/js_json_intro.asp) allows to:
  - use either `:` or `=` between name and value, 
    - (note: `name:value` is a compound work, while `name: value` is a name/value pair),
  - use either `,` or `;` or space as item separator,
  - use either `"` or `'` as quote,
  - avoid quotes for strings without space or any meta-char `=,;[]{}`, 
    - while the meta-char `:` is considered as a part of the word if followed by a letter (it must be quoted if a the end of a word),
  - require a minimal number of space in the input string,
  - accept string with `\\n` line separators (replaced by the "\\n" sequence), also manage `\\b`, `\\r`, `\\t`, `\\f` space chars,
  - set the value `true` for name without explicit value,
  - reads number of the form `0x?????` as hexadecimal numbers,
  - accept end of line comments starting with `#` or `//` and skip them.
- However: 
  - `\\uXXXX` Unicode string sequences and the `\/` solidus escaped sequence are not managed (i.e., but simply mirrored in the string value).

- One consequence is that there is no syntax error all strings map on a JSON structure (i.e., the exact or closest correct JSON structure, the implicit metric being defined by the parsing algorithm).

- To verify that the input was well-formed, one simply has to output the data as a "2D" human readable tabulated indented weak or strict JSON syntax, and read the result.

- On output, this weak syntax also allows to serializes a data structure with a rather minimal number of spaces and meta-character.



