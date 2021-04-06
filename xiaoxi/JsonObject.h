#ifndef FND_JSON_H
#define FND_JSON_H
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>
using ptree = boost::property_tree::ptree;
inline std::string ptreeToJsonString(const ptree & tree){//if不使用inline，不同cpp文件引用会产生多个实体编译不会出错 链接会出错原因函数重复
    std::stringstream ss;
    boost::property_tree::write_json(ss, tree, false);
    return ss.str();
}

#endif