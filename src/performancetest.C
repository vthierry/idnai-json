#if PERFORMANCE_TEST == 1

#include "file.hpp"
#include "time.hpp"
#include <nlohmann/json.hpp>
#include <json/json.h>
#include "WJSONWriter.hpp"

wjson::Value test_data_of(String test)
{
  wjson::Value result;
  printf("test_data_of(\"%s\")\n", test.c_str());
  std::string input = aidesys::load("../public/performancetest/" + test + ".json");
  // test_jsoncpp
  {
    aidesys::now(false);
    Json::Value value;
    {
      Json::CharReaderBuilder builder;
      Json::CharReader *reader = builder.newCharReader();
      std::string errors;
      reader->parse(input.c_str(), input.c_str() + input.size(), &value, &errors);
    }
    result["jsoncpp"]["read"] = (int) rint(aidesys::now(false, true));
    {
      Json::StreamWriterBuilder builder;
      std::string output = Json::writeString(builder, value);
    }
    result["jsoncpp"]["write"] = (int) rint(aidesys::now(false, true));
    Json::Value value2 = value;
    result["jsoncpp"]["copy"] = (int) rint(aidesys::now(false, true));
  }
  // test_lohmann
  {
    aidesys::now(false);
    nlohmann::ordered_json value;
    value = nlohmann::json::parse(input);
    result["lohmann"]["read"] = (int) rint(aidesys::now(false, true));
    value = nlohmann::ordered_json::parse(input);
    result["lohmann"]["ordered_read"] = (int) rint(aidesys::now(false, true));
    std::string output = value.dump();
    result["lohmann"]["write"] = (int) rint(aidesys::now(false, true));
    nlohmann::ordered_json value2 = value;
    result["lohmann"]["copy"] = (int) rint(aidesys::now(false, true));
  }
  // test_wjson
  {
    wjson::WJSONWriter::force_fast = true;
    aidesys::now(false);
    wjson::Value value(input, true);
    result["wjson"]["read"] = (int) rint(aidesys::now(false, true));
    result["count"] = value.count();
    value.check();
    aidesys::now(false);
    std::string output = value.asString();
    result["wjson"]["write"] = (int) rint(aidesys::now(false, true));
    wjson::Value value2 = value;
    result["wjson"]["copy"] = (int) rint(aidesys::now(false, true));
    {
      wjson::Value value(output, true);
      aidesys::alert(value.asString() != output, "  illegal-state", "in test_data wjson spurious read/write/read");
    }
  }
  return result;
}
int main()
{
  wjson::Value results;
  {
    printf("This is a comparative performance test\n");
    printf("\tusing benkmarks from https://github.com/miloyip/nativejson-benchmark\n");
    results["twitter"] = test_data_of("twitter");
    results["canada"] = test_data_of("canada");
    results["citm"] = test_data_of("citm_catalog");
    printf("%s\n", results.asString(true).c_str());
  }
  {
    printf("\nThis is the LaTex generation of a table\n");
    std::string lresults = "\\begin{tabular}{|l|c|c|c|c|c|c|c|c|c|c|c|c|}\\hline&\\multicolumn{3}{c|}{twitter}&\\multicolumn{3}{c|}{canada}&\\multicolumn{3}{c|}{citm}\\\\&read&write&copy&read&write&copy&read&write&copy\\\\\\cline{2-10}\n";
    std::vector < std::string > methods = { "lohmann", "wjson" }, tests = { "twitter", "canada", "citm" }, actions = { "read", "write", "copy" };
    for(auto im = methods.cbegin(); im != methods.cend(); im++) {
      lresults += *im + "&";
      for(auto it = tests.cbegin(); it != tests.cend(); it++) {
        for(auto ia = actions.cbegin(); ia != actions.cend(); ia++) {
          lresults += results.at(*it).at(*im).get(*ia, "") + "&";
        }
      }
      lresults = lresults.substr(0, lresults.size() - 1) + "\\\\\n";
    }
    lresults += "\\hline\\end{tabular}\n";
    printf("%s", lresults.c_str());
  }
  return 0;
}
#else

#include <cstdio>

int main()
{
  printf("Upps performancetest.C is not activated, edit the makefile\n");
}
#endif
