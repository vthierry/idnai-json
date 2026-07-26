#include "Value.hpp"
#include "WJSONReader.hpp"
#include "WJSONWriter.hpp"

#include <vector>
#include <set>
#include <unordered_map>
#include <cmath>

#include "regex.hpp"

namespace wjson {
  const Value Value::EMPTY;
  void Value::Object::set(const Object& object)
  {
    modified = true;
    names = object.names;
    delete values;
    values = NULL;
    if(object.values != NULL) {
      values = new std::unordered_map < std::string, Value > ();
      for(auto it = object.values->begin(); it != object.values->end(); it++) {
        (*values)[it->first] = it->second;
      }
    }
    key_count = object.key_count, index_count = object.index_count, index_max = object.index_max;
  }
  Value& Value::Object::get(String name)
  {
    if(values == NULL) {
      values = new std::unordered_map < std::string, Value > ();
    }
    auto it = values->find(name);
    if(it == values->end()) {
      modified = true;
      {
        char *end;
        int r = 1 + strtol(name.c_str(), &end, 10);
        if(*end == '\0' && (name[0] != '0' || name.size() == 1) && r > 0) {
          if((unsigned int) r > index_max) {
            index_max = r;
          }
          index_count++;
        } else {
          key_count++;
        }
      }
      names.push_back(name);
      return (*values)[name];
    } else {
      return it->second;
    }
  }
  void Value::Object::del(String name)
  {
    if(values != NULL) {
      std::unordered_map < std::string, Value > ::iterator it = values->find(name);
      if(it != values->end()) {
        modified = true;
        values->erase(name);
        std::vector < std::string > ::iterator it = std::find(names.begin(), names.end(), name);
        if(it != names.end()) {
          names.erase(it);
        }
        {
          char *end;
          int r = 1 + strtol(name.c_str(), &end, 10);
          if(*end == '\0' && (name[0] != '0' || name.size() == 1) && r > 0) {
            if((unsigned int) r == index_max) {
              for(std::unordered_map < std::string, Value > ::const_iterator it = values->find(std::to_string(index_max - 1)); 0 < index_max && it == values->end(); index_max--, it = values->find(std::to_string(index_max - 1))) {}
            }
            index_count--;
          } else {
            key_count--;
          }
        }
      }
      if(values->size() == 0) {
        delete values;
        values = NULL;
      }
    }
  }
  void Value::Object::clear()
  {
    modified = true;
    delete values;
    values = NULL;
    names.clear();
    key_count = index_count = index_max = 0;
  }
  Value::Object::~Object()
  {
    delete values;
  }
  Value::Value(String value, bool parse, bool sort_names)
  {
    if(parse) {
      static WJSONReader reader;
      reader.read(value, *this);
      if(sort_names) {
        this->sortNames();
      }
    } else {
      *this = value;
    }
  }
  void Value::updateType() const
  {
    switch(type) {
    case record:
      break;
    case array:
      if(object.key_count > 0 || object.index_count != object.index_max) {
        object.modified = true, type = record;
      }
      break;
    case atomic:
      if(object.values != NULL) {
        object.modified = true, type = object.key_count == 0 && object.index_count == object.index_max ? array : record;
      }
      break;
    }
  }
  std::string Value::asString(bool pretty, bool strict, bool fast) const
  {
    if(!isRecord()) {
      return string;
    } else {
      static WJSONWriter writer;
      if(pretty || strict || fast) {
        return writer.write(*this, pretty, strict, fast);
      } else if(object.modified) {
        as_string = writer.write(*this, pretty, strict, fast);
      }
      return as_string;
    }
  }
  Value& Value::aget(String id, JSON value)
  {
    for(unsigned int i = 0; i < length(); i++) {
      auto it = object.values->find(std::to_string(i));
      if(it != object.values->end() && it->second.at(id).equals(value)) {
        return it->second;
      }
    }
    Value r;
    r.set(id, value);
    return object.values->find(id)->second;
  }
  Value& Value::set(String name, const Value& value)
  {
    if(value.isEmpty()) {
      erase(name);
    } else {
      (*this)[name] = value;
    }
    return *this;
  }
  Value& Value::add(unsigned int index, const Value& value)
  {
    if(value.isEmpty()) {
      // Erases by left shifting
      if(object.index_max > 0) {
        for(unsigned int i = index; i < object.index_max - 1; i++) {
          (*object.values)[std::to_string(i)] = (*object.values).at(std::to_string(i + 1));
        }
      }
      object.del(std::to_string(object.index_max - 1));
    } else {
      // Inserts by right shifting
      for(int i = object.index_max; i > (int) index; i--) {
        set(i, at(i - 1));
      }
      set(index, value);
    }
    return *this;
  }
  unsigned int Value::count() const
  {
    if(isRecord()) {
      unsigned int c = 0;
      for(auto it = getNames().begin(); it != getNames().end(); ++it) {
        c += 1 + at(*it).count();
      }
      return c;
    } else {
      return 0;
    }
  }
  Value& Value::copy(const Value& value)
  {
    for(auto it = value.getNames().begin(); it != value.getNames().end(); ++it) {
      set(*it, value.at(*it));
    }
    return *this;
  }
  bool Value::rename(String old_name, String new_name)
  {
    if(object.values != NULL) {
      for(std::vector < std::string > ::iterator it = object.names.begin(); it != object.names.end(); ++it) {
        if(old_name == *it) {
          *it = new_name;
          (*object.values)[new_name] = object.values->at(old_name);
          object.values->erase(old_name);
          return true;
        }
      }
    }
    return false;
  }
  void Value::sort(bool (*before)(JSON lhs, JSON rhs))
  {
    if(object.values != NULL) {
      std::set < Value, bool (*)(JSON lhs, JSON rhs) > values(before);
      for(unsigned int i = 0; i < length(); i++) {
        values.insert(at(i));
      }
      unsigned int i = 0;
      for(auto it = values.begin(); it != values.end(); it++, i++) {
        (*this)[i] = *it;
      }
    }
  }
  void Value::sortNames()
  {
    struct {
      bool operator()(String lhs, String rhs) {
        char *end_lhs, *end_rhs;
        int v_lhs = strtol(lhs.c_str(), &end_lhs, 10), v_rhs = strtol(rhs.c_str(), &end_rhs, 10);
        if(*end_lhs == '\0') {
          if(*end_rhs == '\0') {
            return v_lhs < v_rhs;
          } else {
            return true;
          }
        } else {
          if(*end_rhs == '\0') {
            return false;
          } else {
            return lhs < rhs;
          }
        }
      }
    }
    before;
    std::sort(object.names.begin(), object.names.end(), before);
    if(object.values != NULL) {
      for(auto it = object.values->begin(); it != object.values->end(); it++) {
        it->second.sortNames();
      }
    }
  }
  bool Value::equals(const Value& value) const
  {
    return Value::asString() == value.Value::asString();
  }
  Value string2json(String string, bool sort_names)
  {
    static WJSONReader reader;
    Value result;
    reader.read(string, result);
    if(sort_names) {
      result.sortNames();
    }
    return result;
  }
  std::string wjson2json(String string)
  {
    static WJSONReader reader;
    Value result;
    reader.read(string, result);
    return result.asString(true, true);
  }
  std::string wjson2wjson(String string)
  {
    static WJSONReader reader;
    Value result;
    reader.read(string, result);
    return result.asString(true);
  }
  std::string quote(String word)
  {
    return word == "" ||
           aidesys::regexMatch(word, "^.*[,;={}[\\]\"\\s].*$") ||
           aidesys::regexMatch(word, "^.*:$") ?
           "\"" + aidesys::regexReplace(word, "\"", "\\\"") + "\"" : word;
  }
  void Value::check(bool dump, bool root)
  {
    std::string s;
    if(object.values != NULL) {
      // We must have object != NULL xor string != ""
      if(string != "") {
        s += "'!{ string: " + string + "} spurious string with object'";
      }
      // If the values buffer is defined it is not empty
      if(object.values->size() == 0) {
        s += "'!spurious empty field buffer'";
      }
      // Names and values must have the same size
      if(object.names.size() != object.values->size()) {
        s += aidesys::echo("'!{names: #%d values: #%d} spurious name counts", object.names.size(), object.values->size());
      }
      // Key/Index counts must correspond to non-numerical/numerical names and Index max to the maximal numerical + 1
      {
        unsigned int key_count = 0, index_count = 0, index_max = 0;
        for(unsigned int i = 0; i < object.names.size(); i++) {
          if(aidesys::regexMatch(object.names[i], "^(0|[1-9][0-9]*)$")) {
            char *end;
            int r = 1 + strtol(object.names[i].c_str(), &end, 10);
            if((unsigned int) r > index_max) {
              index_max = r;
            }
            index_count++;
          } else {
            key_count++;
          }
        }
        if(object.key_count != key_count) {
          s += aidesys::echo("'!{key_count: #%d != #%d} spurious key count'", object.key_count, key_count);
        }
        if(object.index_count != index_count) {
          s += aidesys::echo("'!{index_count: #%d != #%d} spurious index count'", object.index_count, index_count);
        }
        if(object.index_max != index_max) {
          s += aidesys::echo("'!{index_max: #%d != #%d} spurious index max'", object.index_max, index_max);
        }
        if(object.names.size() != key_count + index_count) {
          s += aidesys::echo("'!{names: #%d count: { key: #%d index: #%d} total: #%d} spurious key or index counts", object.names.size(), key_count, index_count, key_count + index_count);
        }
      }
      // All values keys must be in names
      for(auto it = object.values->begin(); it != object.values->end(); it++) {
        auto it2 = std::find(object.names.begin(), object.names.end(), it->first);
        if(it2 == object.names.end()) {
          s += "'!{ name: " + it->first + "} in values but not in names'";
        }
      }
      // All names must be in value keys
      for(unsigned int i = 0; i < object.names.size(); i++) {
        auto it = object.values->find(object.names[i]);
        if(it == object.values->end()) {
          s += "'{name: " + object.names[i] + "} in names but not in values' ";
        }
      }
      // The type must be coherent, if not empty
      if(!isEmpty()) {
        updateType();
        switch(type) {
        case atomic:
          aidesys::alert(object.values != NULL, "illegal-state", "in wjson::Value::check bad atomic type");
          break;
        case array:
          aidesys::alert(object.values == NULL || object.key_count > 0 || object.index_count != object.index_max, "illegal-state", "in wjson::Value::check bad array type { object.values: 0x%lx object.key_count: %d  object.index_count: %d object.index_max: %d}", object.values, object.key_count, object.index_count, object.index_max);
          break;
        case record:
          aidesys::alert(object.values == NULL, "illegal-state", "in wjson::Value::check bad record type");
          break;
        }
      }
      // Recursive check, including cyclic detection
      {
        static std::unordered_map < unsigned long int, bool > addresses;
        if(root) {
          addresses.clear();
        }
        unsigned long int a = (unsigned long int) this;
        aidesys::alert(addresses.find(a) != addresses.end(), "illegal-state", "in wjson::Value::check cyclic storage !!!");
        addresses[a] = true;
        // Recursive check
        {
          for(auto it = object.values->begin(); it != object.values->end(); it++) {
            it->second.check(false, false);
          }
        }
        if(root) {
          // Dumps node size and some elements for manual check
          if(dump) {
            printf("{ wjson::Value  key_count: %d index_count: %d index_count: %d string: '%s' names: [ ",
                   object.key_count, object.index_count, object.index_max, string.c_str());
            for(unsigned int i = 0; i < object.names.size(); i++) {
              printf("'%s' ", object.names[i].c_str());
            }
            printf("] node: %d }\n", (int) addresses.size());
          }
          addresses.clear();
        }
      }
    }
    aidesys::alert(s != "", "illegal-state", "in wjson::Value::check spurious « " + s + " » data structure");
  }
  const std::vector < double >& array2vector(const Value& data, double defaultValue) {
    static std::vector < double > result;
    result.clear();
    for(unsigned int i = 0; i < data.length(); i++) {
      result.push_back(data.get(i, defaultValue));
    }
    return result;
  }
  const std::vector < double >& array2vector(const Value& data, String name, double defaultValue) {
    static std::vector < double > result;
    result.clear();
    for(unsigned int i = 0; i < data.length(); i++) {
      result.push_back(data.at(i).get(name, defaultValue));
    }
    return result;
  }
}
