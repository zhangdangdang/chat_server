#include <sstream>
#include<cstdlib>
#include<cstring>
#include<iostream>
#include "structHeader.h"
#include "SerilizationObject.h"
#include "JsonObject.h"
#include "Protocal.pb.h"
template <typename T> 
std::string seriliaze(const T &obj){
    std::stringstream ss;
    boost::archive::text_oarchive oa(ss);//o是output输出
    oa &obj;
    return ss.str();
}

bool parseMessage(const std::string &input, int *type, std::string &outbuffer){
    auto pos = input.find_first_of(" ");//找第一个 空格的位置
    if(pos==std::string::npos)
        return false;
    if(pos==0)
        return false;
    auto command = input.substr(0, pos);//返回一个从指定位置开始的指定长度的子字符串。
    if(command=="BindName"){
        std::string name = input.substr(pos + 1);
        if(name.size()>32)
            return false;
        if(type)
            *type = MT_BIND_NAME;
        BindName bindInfo;
        bindInfo.nameLen = name.size();
        std::memcpy(&(bindInfo.name), name.data(), name.size());
        auto buffer = reinterpret_cast<const char *>(&bindInfo);//setMessage传入的是const 指针强制转换到 char指向的位置
        outbuffer.assign(buffer, buffer + sizeof(bindInfo));//赋值头部和尾部迭代器
        return true;
    }else if(command=="chat"){
        std::string chat = input.substr(pos + 1);
        if(chat.size()>256)
            return false;
        ChatInformation info;
        info.infoLen = chat.size();
        std::memcpy(&(info.information), chat.data(), chat.size());
        auto buffer = reinterpret_cast<const char *>(&info);//setMessage传入的是const 指针强制转换到
        outbuffer.assign(buffer, buffer + sizeof(info));//赋值
        if(type)
            *type = MT_CHAT_INFO;
        return true;
    }
    return false;
}
bool parseMessage2(const std::string &input, int *type, std::string &outbuffer){
auto pos = input.find_first_of(" ");//找第一个 空格的位置
    if(pos==std::string::npos)
        return false;
    if(pos==0)
        return false;
    auto command = input.substr(0, pos);//返回一个从指定位置开始的指定长度的子字符串。
    if(command=="BindName"){
        std::string name = input.substr(pos + 1);
        if(name.size()>32)
            return false;
        if(type)
            *type = MT_BIND_NAME;
        outbuffer = seriliaze(SBindName(std::move(name)));
        return true;
    }else if(command=="chat"){
        std::string chat = input.substr(pos + 1);
        if(chat.size()>256)
            return false;
        outbuffer = seriliaze(SChatInfo(std::move(chat)));
        if(type)
            *type = MT_CHAT_INFO;
        return true;
    }
    return false;

}
bool parseMessage3(const std::string &input, int *type, std::string &outbuffer){
auto pos = input.find_first_of(" ");//找第一个 空格的位置
    if(pos==std::string::npos)
        return false;
    if(pos==0)
        return false;
    auto command = input.substr(0, pos);//返回一个从指定位置开始的指定长度的子字符串。
    if(command=="bindname"){
        std::string name = input.substr(pos + 1);
        if(name.size()>32)
            return false;
        if(type)
            *type = MT_BIND_NAME;
        //outbuffer = seriliaze(SBindName(std::move(name)));
        ptree tree;
        tree.put("name", name);
        outbuffer = ptreeToJsonString(tree);
        return true;
    }else if(command=="chat"){
        std::string chat = input.substr(pos + 1);
        if(chat.size()>256)
            return false;
        //outbuffer = seriliaze(SChatInfo(std::move(chat)));
        ptree tree;
        tree.put("information", chat);
        outbuffer = ptreeToJsonString(tree);
        if(type)
            *type = MT_CHAT_INFO;
        return true;
    }
    return false;
}
bool parseMessage4(const std::string &input, int *type, std::string &outbuffer){
auto pos = input.find_first_of(" ");//找第一个 空格的位置
    if(pos==std::string::npos)
        return false;
    if(pos==0)
        return false;
    auto command = input.substr(0, pos);//返回一个从指定位置开始的指定长度的子字符串。
    if(command=="bindname"){
        std::string name = input.substr(pos + 1);
        if(name.size()>32)
            return false;
        if(type)
            *type = MT_BIND_NAME;
        //outbuffer = seriliaze(SBindName(std::move(name)));
        //ptree tree;
        //tree.put("name", name);
        //outbuffer = ptreeToJsonString(tree);
        PBindName bindName;
        std::cout << "stru name " << name << std::endl;
        bindName.set_name(name);
        auto ok = bindName.SerializeToString(&outbuffer);//传进区一个指针
        return ok;
    }else if(command=="chat"){
        std::string chat = input.substr(pos + 1);
        if(chat.size()>256)
            return false;
        //outbuffer = seriliaze(SChatInfo(std::move(chat)));
        //ptree tree;
        //tree.put("information", chat);
        //outbuffer = ptreeToJsonString(tree);
        PChat pchat;
        pchat.set_information(chat);
        std::cout << "stru chat " << chat << std::endl;
        auto ok = pchat.SerializeToString(&outbuffer);
        
        if(type)
            *type = MT_CHAT_INFO;
        std::cout << "stru chatMT_CHAT_INFO " << *type << std::endl;
        return ok;
    }
    return false;

}