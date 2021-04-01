#include "structHeader.hpp"
#include<cstdlib>
#include<cstring>
#include<iostream>
bool parseMessage(const std::string &input, int *type, std::string &outbuffer){
    auto pos = input.find_first_of("");//找第一个 空格的位置
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