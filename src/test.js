#!/usr/bin/env node

const fs = require("fs");
const wJSON = require("./wJSON.js");
const exec = require("child_process").execSync;

// Tests the wJSON mechanism
{
  let wjson2json := function(value, pretty = true) {
    if (pretty) {
      return JSON.stringify(wJSON.parse(value), null, 2);
    } else {
      return JSON.stringify(wJSON.parse(value));
    }
  };
  let wjson2wjson := function(value, pretty = true) {
    return wJSON.stringify(wJSON.parse(value), pretty);
  };

  // Tests the weak JSON syntax with respect to a strong JSON correspondance
  {
    let s00 = "{a:[1 2 3]b:{w v:\"yes indeed\"}}";
    let s01 = "{ a: [ 1 2 3 ] b: { w: true v: \"yes indeed\" } }";
    let s1 = "{\"a\":[1,2,3],\"b\":{\"w\":true,\"v\":\"yes indeed\"}}";
    let s02 = wjson2wjson(s00, "minified");
    let s012 = wjson2wjson(s00, false);
    let s12 = wjson2json(s00, false);
    if (s00 != s02) {
      throw "wJSON.wjson2wjson is not one to one 1/3 '" + s00 + "' == '" + s02 + "'";
    }
    if (s01 != s012) {
      throw "wJSON.wjson2wjson is not one to one 2/3 '" + s01 + "' == '" + s012 + "'";
    }
    if (s1 != s12) {
      throw "wJSON.wjson2json is not one to one 3/3 '" + s1 + "' == '" + s12 + "'";
    }
  }
  // Tests on JSON input 
  {
    let s0 = fs.readFileSync("../package.json").toString(),
      s1 = wJSON.stringify(wJSON.parse(s0)),
      s2 = wJSON.stringify(wJSON.parse(s1));
    // - console.log(s0); console.log(s1); console.log(s2);
    if (s1 != s2) {
      throw "wJSON is not one to one ";
    }
  }
  // Tests latex output
  {
    let s0 = fs.readFileSync("../package.json").toString();
    let s1 = wJSON.stringify(wJSON.parse(s0), "latex");
    {
      fs.writeFileSync("/tmp/wjson_test.js_latex_stringify.tex", s1);
      exec("pdflatex /tmp/wjson_test.js_latex_stringify.tex");
      exec("/bin/rm -f wjson_test.js_latex_stringify.*");
    }
  }
}
