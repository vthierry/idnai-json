#ifndef __wjson_cpp2json__
#define __wjson_cpp2json__
#ifndef SWIG

#include "Value.hpp"
#include "regex.hpp"
#include <map>
#include <vector>
#include <opencv2/core/core.hpp>

namespace wjson {
  /**
   * @class cpp2wjson
   * @description This factory defines a wrapper between C++ and wjson data structures.
   * - It is to be called using the `wjson::cpp2wjson::get(...)` construct.
   * - This is (indeed) not available from non C/C++ language wrappers.
   */
  class cpp2wjson {
public:

    /**
     * @function get
     * @memberof cpp2wjson
     * @static
     * @description Gets a JSON representation from a compatible data structure.
     * - It is the responsability of the user to call this template static method with compatible types.
     * @param {vector|map|pair|Mat|Value|string|double|int|uint|bool} value A mappable value to wjson value.
     * - Here, `std::vector` and `std::pair` is returned as lists.
     * - Here, `std:map` is returned as record with key converted to string.
     * - The conversion of [opencv `cv::Mat`](https://docs.opencv.org/4.8.0/d3/d63/classcv_1_1Mat.html) is also implemented.
     * @return {Value} The obtained `wjson::Value` value.
     */
    template < typename T > wjson::Value static get(const std::vector < T > &value)
    {
      wjson::Value result;
      for(auto it = value.cbegin(); it != value.cend(); it++) {
        result.add(get(*it));
      }
      return result;
    }
    template < typename S, typename T > wjson::Value static get(const std::map < S, T > &value)
    {
      wjson::Value result;
      for(auto it = value.cbegin(); it != value.cend(); it++) {
        result.set(it->first, it->second);
      }
      return result;
    }
    template < typename S, typename T > static wjson::Value get(const std::pair < S, T > &value)
    {
      wjson::Value result;
      result.add(get(value.first));
      result.add(get(value.second));
      return result;
    }
    static wjson::Value get(const cv::Mat& mat)
    {
      std::ostringstream stream;
      stream << mat;
      return wjson::string2json(aidesys::echo("{type: Mat rows: %d cols: %d values: [\n  ", mat.rows, mat.cols) + aidesys::regexReplace(aidesys::regexReplace(stream.str(), "]", "]\n ]}"), ";\n ", "]\n  ["));
    }
    template < typename T > static wjson::Value get(const T& value)
    {
      return value;
    }
    
    /**
     * @function get
     * @memberof cpp2wjson
     * @static
     * @description Gets a std::map < S, T >  from a wjson data structure.
     * - .
     * @param {Value} value The input value.
     * @param {S} key A key of the `std::map` expected type to allow the compiler to infer the `S` type.
     * @param {T} item A value of the `std::map` expected type to allow the compiler to infer the `T` type.
     * @return {map} The obtained `std::map` value.
     */
    template < typename S, typename T > std::map < S, T > static get(JSON value, const S& key, const T& item) {
      std::map < S, T > result;
      for(auto it = value.getNames().cbegin(); it != value.getNames().cend(); ++it) {
        result[*it] = value.at(*it);
      }
      return result;
    }

    /**
     * @function get
     * @memberof cpp2wjson
     * @static
     * @description Gets a std::vector < T >  from a wjson data structure.
     * - .
     * @param {Value} value The input value.
     * @param {T} item A value of the `std::vector` expected type to allow the compiler to infer the `T` type.
     * @return {vector} The obtained `std::vector` value.
     */
    template < typename S, typename T > std::vector < T > static get(JSON value, const T& item) {
      std::vector < T > result;
      for(unsigned int i = 0; i < value.length(); i++) {
        result.push_back(value.at(i));
      }
      return result;
    }
  };
}
#endif
#endif
