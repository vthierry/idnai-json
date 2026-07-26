#ifndef __wjson_Value__
#define __wjson_Value__

#include "std.hpp"
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace wjson {
  class WJSONReader;

  /**
   * @class Value
   * @description Implements a minimal C/C++ [JSON](https://www.json.org) data structure with weak-syntax reader/writer.
   * - Such a value is either
   *   - an atomic string
   *     - including string parsable as numeric or boolean value, or
   *   - a [record](https://en.wikipedia.org/wiki/Record_%28computer_science%29), i.e., a recursive construct of,
   *     - an unordered set of value accessed by label, while
   *     - an array is a record {\tt \{ 0: a 1: b ...\}} indexed by consecutive non negative integer.
   * - Value functions
   *   - Element access:
   *     - [`value.isMember(name)`](#isMember) allows to check if a value is defined,
   *     - [`value.at(name)`](#at) allows to access a value as a record,
   *       - the at()` is virtual, i.e, can be overloaded allowing to define non-memorized dynamic calculated values.
   *     - [`value.get(name, default_typed_value)`](#get) allows to access an atomic value.
   *   - Modifiers:
   *     - [`value.set(name, value)`](#set) allows to write a value,
   *     - [`value.erase(name)`](#erase) allows to erase a value,
   *     - [`value.clear()`](#clear) allows to empty the value.
   *   - Capacity and property:
   *     - [`value.isEmpty()`](#isEmpty) allows to check if the value is empty,
   *     - [`value.isRecord()`](#isRecord) allows to check if the value is a record,
   *     - [`value.isArray()`](#isArray) allows to check if the value is an array,
   *     - [`value.getNames()`](#getNames) returns all the record field names, allowing to iterate on the fields,
   *     - [`value.size()`](#size) returns the number of records,
   *     - [`value.length()`](#length) returns the length of the value as an array, allowing to iterate on the array elements.
   *   - Other operations:
   *     - [`value.asString()`](#asString) returns this value as a string,
   *     - [`value.equal(another_value)`](#equal) checks if two values are equal,
   *     - [`value.sortNames()`](#sortNames) allows to sort the record field names in alphabetic order,
   *     - [`value.sort(before)`](#sort) allows to sort an array,
   * - Note: The Value is equipped with a equal and less operators and a hash function in order to be used in containers such as `std::unordered_map<wjson::Value, T>` or `std::map<wjson::Value, T>`; however in the latter case the order is simply the lexical order of the string representation which is likely semantically biased. A correct semantic order depends on the data type as defined in the [symboling package](https://line.gitlabpages.inria.fr/aide-group/symboling/Type.html).
   * - Note: `JSON` is a read-only reference, i.e. an alias of `const wjson::Value&` for readability convenience.
   * @param {string|bool|int|uint|double|float|JSON} [value] The initial value.
   * @param {bool} [parse=false] For a string value, if true parses the string, else only copy it.
   * @param {bool} [sort_names=false] For a parsed string value, if true sorts the names in alphabetic order, as performed by sortNames().
   *
   * __Note__: The [ccp2wjson](https://gitlab.inria.fr/line/aide-group/wjson/-/raw/master/src/cpp2wjson.tpp)  factory defines a wrapper between C++ and wjson data structures.
   */
  class Value {
    // This value atomic value
    std::string string;
    // Used to manage empty values
    enum Type {
      atomic, array, record
    }
    mutable type = atomic;
    friend WJSONReader;
    // Updates the record versus array type if required
    void updateType() const;
    // This value indexed values
    class Object {
      friend class wjson::Value;
      // Names in insertion order
      std::vector < std::string > names;
      // Values table
      std::unordered_map < std::string, Value > *values = NULL;
      // Maximal number of numerical indexes
      unsigned int index_count = 0, index_max = 0;
      // Number of non numerical keys
      unsigned int key_count = 0;
      // Flag to register if the value is modified
      mutable bool modified = true;
      // Sets this object from another object
      void set(const Object& object);
      // Gets a value reference creating the entry if not available
      Value& get(String name);
      // Deletes an entry
      void del(String name);
      // Clears this object
      void clear();
      // Bye :)
      virtual ~Object();
    };
    Object object;
public:
    Value() {}
    Value(const Value& value) {
      *this = value;
    }
    Value(String value, bool parse, bool sort_names = false);
    template < typename T > Value(T value) {
      *this = value;
    }
    virtual ~Value() {}

    /**
     * @function asString
     * @memberof Value
     * @instance
     * @description Returns the data structure as a string.
     * - This allows to check if two data structures are equal for the modified semantic:
     *   - Two literal values are equal if and only the literal string value `asString()` are equal, see equals().
     *   - Two data structures are equal if and only each item are inserted in the same order and equal.
     * @param {bool} [pretty=false] If true properly format in 2D, else returns a minimal raw format.
     * @param {bool} [strict=false] If true [strict JSON](https://www.json.org/json-en.html) syntax, else produces a [weak JSON](./wJSON.html#.parse) syntax.
     * @param {bool} [fast=false] If fast serializes the data as fast as possible (quoting all words without any lexical analysis).
     * @return {string} The value as a string.
     */
    std::string asString(bool pretty = false, bool strict = false, bool fast = false) const;
private:
    mutable std::string as_string = "";
public:

    /**
     * @function isEmpty
     * @memberof Value
     * @instance
     * @description Returns true if this structure is empty, i.e., without any field not literal value.
     * - As a literal, the empty value casts to the empty string `""`, the boolean `false` value, the integral number `0`, or the floating point number `NAN`.
     * - An empty record corresponds to parsing `{}` and an empty array to parsing `[]`.
     * - The `clear()` methods allows to clear all value content.
     * @return {bool} The check result.
     */
    bool isEmpty() const
    {
      return object.values == NULL && string.size() == 0;
    }
    /**
     * @member {Value} EMPTY
     * @memberof Value
     * @static
     * @description The `wjson::Value::EMPTY` is a generic read-only empty value.
     */
    static const Value EMPTY;

    /**
     * @function isRecord
     * @memberof Value
     * @instance
     * @description Returns true if this structure is a non empty record, false otherwise.
     * - A value is a record as soon as a field as been defined using the subscript `[]` operator.
     * - By contract an array corresponds to a record with numerical keys. It is thus a record.
     * - An object is atomic if and only if it is not a record.
     * @return {bool} The check result.
     */
    bool isRecord() const
    {
      updateType();
      return type != atomic;
    }
    /**
     * @function isArray
     * @memberof Value
     * @instance
     * @description Returns true if this structure is an array, false otherwise.
     * @return {bool} Returns true if it is a record with only consecutive non negative integral numerical keys of syntax `(0|[1-9][0-9]*)`.
     */
    bool isArray() const
    {
      updateType();
      return type == array;
    }
    /**
     * @function isMember
     * @memberof Value
     * @instance
     * @description Returns true if this structure has the given field defined, false otherwise.
     * @param {string|uint} name The field key name or numerical index.
     * @return {bool} Returns true of defined.
     */
    bool isMember(String name) const
    {
      return object.values != NULL && object.values->find(name) != object.values->end();
    }
    bool isMember(unsigned int name) const
    {
      return isMember(std::to_string(name));
    }
    /**
     * @function length
     * @memberof Value
     * @instance
     * @description Returns the number of numerically indexed fields.
     * - The size of the record, i.e., the number of fields is returned by `size()`.
     * ```
     * // Given a Value array the indexed field iterator writes:
     * for(unsigned int i = 0; i < value.length(); i++) {
     *   const Value& field_value = value.at(i);
     *   Value& field_value = value[i];
     * ../..
     * ```
     * @return {uint} The numerical indexed field count.
     */
    unsigned int length() const
    {
      return object.index_max;
    }
    /**
     * @function count
     * @memberof Value
     * @instance
     * @description Returns the number of all attributes of the value and sub-values.
     * @return {uint} The numerical indexed field count.
     */
    unsigned int count() const;

    /**
     * @function getNames
     * @memberof Value
     * @instance
     * @description Returns the field names in the insertion order.
     * ```
     * // Given a Value record the structure field names iterator writes:
     * for(auto it = value.getNames().cbegin(); it != value.getNames().cend(); ++it) {
     *   std::string field_name = *it;
     *   const Value& field_value = value.at(field_name);
     *   Value& field_value = value[field_name];
     * ../..
     * ```
     * @return {Array<string>} A `std::vector<std::string>` with the field key names or index in insertion order.
     */
    const std::vector < std::string >& getNames() const {
      return object.names;
    }

    /**
     * @function size
     * @memberof Value
     * @instance
     * @description Returns the number of fields with either literal or numerical index.
     * - If the record is an array, its length, i.e., the number of fields with numerical indexes is returned by `length()`.
     * - This is a simple shortcut for ``getNames().size()`.
     * @return {uint} The numerical indexed field count.
     */
    unsigned int size() const
    {
      return getNames().size();
    }
    /**
     * @function get
     * @memberof Value
     * @instance
     * @description Returns the literal value of a given field of this structure.
     * ```
     *  wjson::Value value("{ number: 3.1416, no: FaLsE }", true);
     *  double v1 = value.get("number", NAN); // Returns 3.1416 up to the machine precision, since the value is parsable as a double.
     *  double v2 = value.get("no", NAN); // Returns the default value NAN, since the value is not parsable as a double.
     *  float v3 = value.get("number", 0.0f); // Returns 3.1416 up to the machine precision, since the value is parsable as a float.
     *  float v4 = value.get("no", 0.0f); // Returns 0.0, since the value is not parsable as a float.
     *  int v5 = value.get("number", 0); // Returns 3 since the value is numerically parsable and rounds to the integer value 3.
     *  bool v6 = value.get("no", true); // Returns false since the value parses as a boolean.
     *  bool v7 = value.get("number", true); // Returns the default value true since the value is not parsable as a boolean.
     * ```
     * @param {string|uint} [name] The field key name or numerical index.
     * - If the field key is omitted, returns the literal value of this value.
     * @param {string|bool|int|uint|double|float|char|Value} default_value A default value, defining also the field type.
     * @return {string|bool|int|double|double|float|char|Value} The value with the type corresponding to the default value.
     * - If the field is undefined, unparsable or out of range, the default value is returned.
     * - A numeric value is parsed as a double and rounded if returned as an integer.
     * - If the type is `bool` the string is converted to lower-case and valid if equal to "true" or "false".
     * - If the type is `char` the first char of the string value is considered.
     */
    template < typename S, typename T > T get(S name, T default_value) const
    {
      return at(name).get(default_value);
    }
    template < typename S > std::string get(S name, const char *default_value) const
    {
      return at(name).get((std::string) default_value);
    }
    template < typename S > Value get(S name, const Value& default_value) const
    {
      return isMember(name) ? at(name) : default_value;
    }
    std::string get(String default_value) const
    {
      return string == "" ? default_value : string;
    }
    std::string get(const char *default_value) const
    {
      return get((std::string) default_value);
    }
    char get(char default_value) const
    {
      return get(std::to_string(default_value))[0];
    }
    bool get(bool default_value) const
    {
      std::string result = string;
      std::transform(result.begin(), result.end(), result.begin(), ::tolower);
      return result == "true" ? true : result == "false" ? false : default_value;
    }
    int get(int default_value) const
    {
      double result = get((double) default_value);
      return result == NAN ? default_value : (int) rint(result);
    }
    unsigned int get(unsigned int default_value) const
    {
      double result = get((double) default_value);
      return result == NAN || result < 0 ? default_value : (unsigned int) rint(result);
    }
    double get(double default_value) const
    {
      if(string.size() == 0) {
        return default_value;
      } else {
        char *end;
        double r = strtod(string.c_str(), &end);
        return *end == '\0' ? r : default_value;
      }
    }
    float get(float default_value) const
    {
      return (float) get((double) default_value);
    }
    /**
     * @function at
     * @memberof Value
     * @instance
     * @description Returns a given field of this structure.
     * ```
     *   Value value = jsonValue.at(name_or_index); // Allows to get a read-only version of a field value.
     *   Value value = jsonValue[name_or_index];    // Allows to get a field value, and create if not yet defined.
     * ```
     * - This method is virtual, it thus can be overloaded to define non-memorized dynamic calculated values.
     * @param {string|uint} name The field key name or numerical index.
     * - The recursive construct `field-name/sub-field-name` allows to directly access sub-field values.
     *   - Name of the form of an URL, e.g., `http://...` are escaped, because:
     *     - The `//` or `:/` patterns are escaped.
     *     - The `/` char at the first or last location is escaped.
     * @return {Value} The field value as a read-only structured value. If undefined, returns an empty data structure.
     */
    virtual const Value& at(String name) const
    {
      if(object.values == NULL) {
        return EMPTY;
      } else {
        const char *s = name.c_str();
        long int d = (long int) strchr(s, '/') - (long int) s;
        if(0 < d && d < (int) name.size() - 1 && name.at(d - 1) != ':' && name.at(d + 1) != '/') {
          return at(name.substr(0, d)).at(name.substr(d + 1));
        } else {
          std::unordered_map < std::string, Value > ::iterator it = object.values->find(name);
          return it == object.values->end() ? EMPTY : it->second;
        }
      }
    }
    const Value& at(const char *name) const
    {
      return at((std::string) name);
    }
    template < typename S > const Value& at(S name) const
    {
      return at(std::to_string(name));
    }
    /**
     * @function aget
     * @memberof Value
     * @instance
     * @description Returns an indexed record in a array.
     * - In an array of the form `[ { id: value ... } ... ]` returns the first record which id has the given value.
     * - If such a record does not exist appends it. Thus, if not an array inserts the `{ id: value ... }` at index 0.
     * @param {string} id The identificator name.
     * @param {Value} value The value to equa.
     * @return {Value} The record which id has the given value.
     */
    Value& aget(String id, const Value& value);

    /**
     * @function set
     * @memberof Value
     * @instance
     * @description Sets a given field of this structure.
     * - Setting a value:
     * ```
     *   jsonValue.set(name_or_index, value);          // Allows to set a non-empty field value.
     *   jsonValue[name_or_index] = value;             // Allows to set a non-empty or empty field value.
     * ```
     * - Unsetting a value:
     *   - Note the semantic difference between the `set()` method and the `[]` operator
     * ```
     *   jsonValue.erase(name_or_index);               // Allows to unset a field value.
     *   jsonValue.set(name_or_index, EMPTY);          // Allows to unset a field value.
     *   jsonValue[name_or_index] = EMPTY;             // Allows to explicitly set a field to the EMPTY value.
     * ```
     * - Appending a value to an array:
     * ```
     *   jsonValue[jsonValue.length()] = value;        // Allows to append a field value to the jsonValue array.
     *   jsonValue.add(value);                         // Allows to append a field value to the jsonValue array.
     * ```
     * while the add() method allows also to insert/delete value in an array.
     * A construct of the following form allows to set a field value on a temporary clone of a read-only value, thus not modifying the value.
     * ```
     *   Value& otherValue = jsonValue.clone().set(name_or_index, value);
     *```
     * @param {string|uint} name The field key name or numerical index.
     * @param {Value} value The field value. The value is copied.
     * @return {Value} A reference to this value, allowing to chain set methods:
     * ```
     *  value.set(name_or_index, value).set(another_name_or_index, another_value);
     * ```
     */
    Value& set(String name, const Value& value);
    Value& set(const char *name, const Value& value)
    {
      return set((std::string) name, value);
    }
    template < typename S > Value& set(S name, const Value& value)
    {
      return set(std::to_string(name), value);
    }
#ifndef SWIG
    Value& operator[] (String name) {
      if(object.values == NULL) {
        string = "";
      }
      return object.get(name);
    }
    Value& operator[] (const char *name) {
      return (*this)[(std::string) name];
    }
    template < typename S > Value & operator[] (S name) {
      return (*this)[std::to_string(name)];
    }
    Value& operator = (const Value& value) {
      if(this != &value) {
        type = value.type;
        string = value.string;
        object.set(value.object);
      }
      return *this;
    }
    Value& operator = (String value) {
      object.clear();
      this->string = value;
      return *this;
    }
    Value& operator = (const char *value) {
      object.clear();
      this->string = value;
      return *this;
    }
    Value& operator = (char value) {
      object.clear();
      this->string = value;
      return *this;
    }
    Value& operator = (bool value) {
      object.clear();
      this->string = value ? "true" : "false";
      return *this;
    }
    Value& operator = (double value) {
      object.clear();
      if(rint(value) == value) {
        this->string = std::to_string((int) value);
      } else {
        char s[256];
        sprintf(s, "%12g", value);
        this->string = s;
      }
      return *this;
    }
    template < typename T > Value& operator = (T value) {
      object.clear();
      this->string = std::to_string(value);
      return *this;
    }
#endif
    Value clone() const
    {
      return *this;
    }
    /**
     * @function add
     * @memberof Value
     * @instance
     * @description Adds, inserts or deletes a value in this data structure viewed as a list.
     * - Setting a value:
     * ```
     *   jsonValue.add(index, value);          // Allows to insert a value at this index, shifting all sub-sequence values to the right.
     *   jsonValue.add(index, EMPTY);          // Allows to delete a value at this index, shifting all sub-sequence values to the left.
     *   jsonValue.add(value);                 // Allows to append a value
     * ```
     * A construct of the following form allows to set a field value on a temporary clone of a read-only value, thus not modifying the value.
     * ```
     *   Value& otherValue = jsonValue.clone().add(name_or_index, value);
     *```
     * @param {uint} index The numerical index.
     * @param {Value} value The field value. The value is copied.
     * @return {Value} A reference to this value, allowing to chain set methods:
     * ```
     *  value.add(value).set(another_name_or_index, another_value);
     * ```
     */
    Value& add(unsigned int index, const Value& value);
    Value& add(const Value& value)
    {
      return add(object.index_max, value);
    }
    /**
     * @function copy
     * @memberof Value
     * @instance
     * @description Copies all fields of a record or array.
     * @param {Value} value The record or array field value. The value is copied.
     * @return {Value} A reference to this value, allowing to chain set methods.
     */
    Value& copy(const Value& value);

    /**
     * @function erase
     * @memberof Value
     * @instance
     * @description Erases a given field.
     * - When erased the corresponding value is an empty data structure.
     * @param {string|uint} [name] The field key name or numerical index.
     */
    void erase(String name)
    {
      object.del(name);
    }
    void erase(const char *name)
    {
      erase((std::string) name);
    }
    template < typename S > void erase(S name)
    {
      erase(std::to_string(name));
    }
    /**
     * @function rename
     * @memberof Value
     * @instance
     * @description Renames a given field without changing the field order.
     * @param {string} old_name The old field key name.
     * @param {string} new_name The new field key name.
     * @return {bool} True if the old name has been found, false otherwise.
     */
    bool rename(String old_name, String new_name);

    /**
     * @function clear
     * @memberof Value
     * @instance
     * @description Erases all fields and literal content of this value.
     * @param {bool} [preserveType=false]
     * - If true a record becomes an empty record `{}` and an array an empty array `[]`.
     * - Otherwise the object becomes atomic.
     */
    void clear(bool preserveType = false)
    {
      if(!preserveType) {
        type = atomic;
      }
      string = "";
      object.clear();
    }
    /**
     * @function sort
     * @memberof Value
     * @instance
     * @description Sorts the numerically indexed values according to the comparison function.
     * - Only applies on numerically indexed values: Key order, and named value are not affected.
     * ```
     * struct Before {
     *   // Sorts in numerical order considering values as unsigned int
     *   static bool before(JSON lhs, JSON rhs) {
     *     return lhs.get(0u) < rhs.get(0u);
     *   }
     * };
     * jsonValue.sort(Before::before);
     * ```
     * @param {function} before A `static bool before(JSON lhs, JSON rhs)`
     * - which returns `true` if the first argument is less than (i.e., is ordered before) the second.
     */
    void sort(bool (*before)(const Value& lhs, const Value& rhs));

    /**
     * @function sortNames
     * @memberof Value
     * @instance
     * @description Sorts the names and indexes in alphabetic order.
     * - This allows two values with the same field's value to be syntactically equal.
     * - Numerical indexes are sorted in numerical order and non numerical indexes in alphabetic orders, and sorted after the numerical indexes.
     * - This applies recursively on sub-values.
     */
    void sortNames();

    /**
     * @function equals
     * @memberof Value
     * @instance
     * @description Returns true if this date equals the given value.
     *  - Two literal values are equal if and only the literal string value are equal, see equals().
     * - Also available using the `==` or `!=` operator syntax.
     * @param value The value to compare with.
     * @return True if both values are equal.
     */
    bool equals(const Value& value) const;
#ifndef SWIG
    bool operator == (const Value& value) const {
      return Value::asString() == value.Value::asString();
    }
    bool operator != (const Value& value) const {
      return Value::asString() != value.Value::asString();
    }
    bool operator < (const Value& value) const {
      return Value::asString() < value.Value::asString();
    }
#endif

    // Checks the value internal data integrity, used for debug,
    // - if dump prints some elements during check,
    // - if root recursively checks the structure and detects cycling elements.
    void check(bool dump = false, bool root = true);
  };

  /**
   * @function quote
   * @static
   * @description Returns a wjson string name or value with `"` quotes if required, to avoid syntax error.
   * - Called with the `wjson::quote(word)` construct.
   * @param {string} word The word to quote if required.
   * @return {string} Either the word itself or with quote, if appropriate.
   */
  std::string quote(String word);

  /**
   * @function string2json
   * @static
   * @description Static C/C++ string to JSON conversion for convenience.
   * ```
   * // Typical usage is for constructor or method arguments of the form, e.g.:
   * … set(wjson::string2json(aidesys::echo("{do: grab device: %d}", device), …
   *
   * ```
   * @param {String} string The initial value.
   * @param {bool} [sort_names=false] If true sorts the names in alphabetic order, as performed by sortNames()
   * @return {JSON} A temporary reference towards the best effort parsed value.
   */
  wjson::Value string2json(String string, bool sort_names = false);
  std::string wjson2json(String string);
  std::string wjson2wjson(String string);

  /**
   * @function array2vector
   * @static
   * @description Converts a data structure to a numerical array.
   * @param {JSON} data The data structure array of the form:
   * ```
   * [
   *   { name: value … }
   * ../..
   * ]
   * ```
   * or simply:
   * ```
   * [
   *   value
   * ../..
   * ]
   * ```
   * @param {String} [name] An optional field numerical value, if omitted the data structure is assumed to an numerical array.
   * @param {double} [defaultValue = NAN] An optional default value if the expected value is missing.
   * @return {Array} A temporary reference to a `std::vector<double>` array with the related values.
   */
  const std::vector < double >& array2vector(const Value& data, double defaultValue = NAN);
  const std::vector < double >& array2vector(const Value& data, String name, double defaultValue = NAN);
}
#ifndef SWIG
// Defines a hash function to use Value in unordered_map container
template < > struct std::hash < wjson::Value > {
  size_t operator()(const wjson::Value & value) const {
    return std::hash < std::string > {}
           (value.asString());
  }
};
#endif

typedef const wjson::Value& JSON;

#endif
