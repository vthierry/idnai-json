#include "Value.hpp"
#include "regex.hpp"
#include <cstring>
#include <cmath>
#include <map>

int main(int argc, char *argv[])
{
  std::string s00 = "{0: zero 1: un a:[1 2 3]b:{u v:\"yes indeed\"}}";
  std::string s0 = "{ 0: zero 1: un a: [ 1 2 3 ] b: { u v: \"yes indeed\" } }";
  std::string s1 = "{\"0\":\"zero\",\"1\":\"un\",\"a\":[1,2,3],\"b\":{\"u\":true,\"v\":\"yes indeed\"}}";
  // Compares the weak JSON syntax with respect to a strong JSON correspondance
  {
    wjson::Value j00(s00, true), j0(s0, true);
    j00.check();
    std::string s02 = j00.asString();
    std::string s12 = j0.asString(false, true);
    aidesys::alert(s0 != s02, "  illegal-state", "in wjson_test/wjson #1 '" + s0 + "' != '" + s02 + "'");
    aidesys::alert(s1 != s12, "  illegal-state", "in wjson_test/json #2 '" + s1 + "' != '" + s12 + "'");
    // - printf(j0.asString(true).c_str()); // used to manually check the pretty print
  }
  // Tests some set/get functions
  {
    wjson::Value k0, k1, k2, k3;
    k2["u"] = true, k2["v"] = "yes indeed";
    k1["0"] = 3, k1["1"] = 1, k1[2] = 2;
    k3 = k3.clone().set("0", 3).set("1", 1).set(2, 2);
    aidesys::alert(k1.asString() != k3.asString(), "  illegal-state", "in wjson_test/wjson #3 '" + k1.asString() + "' != '" + k3.asString() + "'");
    struct Before {
      static bool before(JSON lhs, JSON rhs)
      {
        return lhs.get(0u) < rhs.get(0u);
      }
    };
    k1.sort(Before::before);
    k0[0] = "zero", k0[1] = "un";
    k0["a"] = k1, k0["b"] = k2;
    k0.check();
    std::string s01 = k0.asString();
    aidesys::alert(s0 != s01, "  illegal-state", "in wjson_test/wjson #3 '" + s0 + "' != '" + s01 + "'");
    // Also test equality using the `==` operator
    wjson::Value j0(s0, true);
    aidesys::alert(j0 != k0, "  illegal-state", "in wjson_test/wjson #3 '" + s0 + "' != '" + s01 + "'");
    aidesys::alert(!(j0 == k0), "  illegal-state", "in wjson_test/wjson #3 '" + s0 + "' != '" + s01 + "'");
    // Further test get operations
    {
      wjson::Value value("{ number: 3.1416, no: FaLsE }", true);
      double v1 = value.get("number", NAN); // Returns 3.1416 since the value is parsable as a double.
      double v2 = value.get("no", NAN); // Returns the default value NAN since the value is not parsable as a double.
      float v3 = value.get("number", 0.0f); // Returns 3.1416 since the value is parsable as a float.
      float v4 = value.get("no", 0.0f); // Returns 3.1416 since the value is parsable as a float.
      int v5 = value.get("number", 0); // Returns 3 since the value is numerically parsable and rounds to the integer value 3.
      bool v6 = value.get("no", true); // Returns false since the value parses as a boolean.
      bool v7 = value.get("number", true); // Returns the default value true since the value is not parsable as a boolean.

      aidesys::alert(fabs(v1 - 3.1416) >= 1e-6 ||
                     !std::isnan(v2) ||
                     fabs(v3 - 3.1416) >= 1e-6 ||
                     v4 != 0.0 ||
                     v5 != 3 ||
                     v6 != false ||
                     v7 != true, "  illegal-state", "in wjson_test/wjson #3.2 v1=%f, v2=%f, v3=%f, v4=%f, v5=%d, v6=%d, v7=%d", v1, v2, v3, v4, v5, v6, v7);
    }
    // Tests recursive sub-field access
    {
      wjson::Value k4("{0: zero 1: un a:[1 2 3] b:{u v: w}", true);
      aidesys::alert(k4.get("b/v", "") != "w", "  illegal-state", "in wjson_test/wjson #3.3 '%s' != 'w'\n", k4.get("b/v", "").c_str());
    }
    // Tests the aget function
    {
      wjson::Value k4("[ {i: 0 j: yes} { i: 1 j: no } { i: 2 j: maybe} ]", true), k5("{ i: 1 j: no }", true);
      aidesys::alert(k4.aget("i", 1) != k5, "  illegal-state", "in wjson_test/wjson #3.4 '%s' != 'w'\n", k4.aget("i", 1).asString().c_str(), k5.asString().c_str());
    }
  }
  // Tests the length() function
  {
    wjson::Value k3("[a b c d]", true);
    k3.erase(0), k3.erase(2), k3.erase(3);
    k3.check();
    aidesys::alert(k3.length() != 2, "  illegal-state", "in wjson_test/wjson #4 count = %d != 2", k3.length());
  }
  // Tests some add functions
  {
    wjson::Value k4, k40("[ one two three four five six seven]", true);
    k4.set(0, "one").add("two").add("two").add("four").add("four").add("six").add(6, "seven").add("none").add(1, wjson::Value::EMPTY).add(2, "three").add(3, wjson::Value::EMPTY).add(4, "five").add(7, wjson::Value::EMPTY);
    k4.check();
    aidesys::alert(k4 != k40, "  illegal-state", "in wjson_test/wjson #5 '" + k4.asString() + "' != '" + k40.asString() + "'");
  }
  // Tests the sortName function
  {
    std::string s01 = "{b:{v:\"yes indeed\" u}a:[1 2 3]0: zero 1: un}";
    wjson::Value j01(s01, true);
    j01.sortNames();
    j01.check();
    std::string s1 = j01.asString();
    aidesys::alert(s0 != s1, "  illegal-state", "in wjson_test/wjson sortNames '" + s0 + "' != '" + s1 + "'");
  }
  // Tests empty objects
  if(false) {
    wjson::Value e0, e01("[]"), e02("{}"), e1("[]", true), e2("{}", true);
    e0.check(), e01.check(), e02.check(), e1.check(), e2.check();
    aidesys::alert(e0.asString() != "\"\"" ||
                   e01.asString() != "\"[]\"" ||
                   e02.asString() != "\"{}\"" ||
                   e1.asString() != "[ ]" ||
                   e2.asString() != "{ }", "  illegal-state", "in wjson_test/wjson emptyObjects { e0: '%s' #%d%d e01: '%s' #%d%d e02: '%s' #%d%d e1: '%s'  #%d%d e2: '%s' #%d%d}",
                   e0.asString().c_str(), e0.isArray(), e0.isRecord(),
                   e01.asString().c_str(), e01.isArray(), e02.isRecord(),
                   e02.asString().c_str(), e01.isArray(), e02.isRecord(),
                   e1.asString().c_str(), e1.isArray(), e1.isRecord(),
                   e2.asString().c_str(), e2.isArray(), e2.isRecord());
  }
  // Tests that Value can be used in map and unordered_map
  {
    {
      std::unordered_map < wjson::Value, bool > isThere;
      wjson::Value value1("{a: 1 b: 2 c:[u,v,w]}", true), value2("{a: 1 b: 2 c: 3}", true), value3("{a: 1 b: 2}", true);
      isThere.insert_or_assign(value1, false);
      isThere.insert_or_assign(value2, false);
      isThere.insert_or_assign(value1, true);
      aidesys::alert(!(isThere.count(value1) == 1 && isThere.count(value3) == 0 && isThere.at(value1) && (!isThere.at(value2))), "  illegal-state", "in wjson_test/unordered_map bad state");
    }
    {
      std::map < wjson::Value, bool > isThere;
      wjson::Value value1("{a: 1 b: 2 c:[u,v,w]}", true), value2("{a: 1 b: 2 c: 3}", true), value3("{a: 1 b: 2}", true);
      isThere.insert_or_assign(value1, false);
      isThere.insert_or_assign(value2, false);
      isThere.insert_or_assign(value1, true);
      aidesys::alert(!(isThere.count(value1) == 1 && isThere.count(value3) == 0 && isThere.at(value1) && (!isThere.at(value2))), "  illegal-state", "in wjson_test/unordered_map bad state");
      value1.check(), value2.check(), value3.check();
    }
  }
}
