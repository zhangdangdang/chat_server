#ifndef chat_MESSAGE
#define chat_MESSAGE
#include "structHeader.h"
#include <cstdlib>
#include <cstdio>
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>
#include "JsonObject.h"

class chat_message{
    public:
        enum
        {
            header_length = sizeof(Header)//长度是8 两int
        };
        enum
        {
            max_body_length = 512
        };
        chat_message(){}

        const char *data() const { return data_; }
        char *data()  { return data_; }
        std::size_t length() const { return header_length + m_header.bodySize; }
        const char *body() const { return data_ + header_length; }
        char *body()  { return data_ + header_length; }
        int type() const { return m_header.type; }
        std::size_t body_length()const {return m_header.bodySize;}
        void setMessage(int messageType,const void *buffer, size_t bufferSize){
            assert(bufferSize <= max_body_length);
            m_header.bodySize = bufferSize;
            m_header.type=messageType;
            memcpy(body(), buffer, bufferSize);
            memcpy(data(), &m_header, sizeof(m_header));
        }
        void setMessage(int messageType,const std::string& buffer){
            setMessage(messageType, buffer.data(), buffer.size());
        }

        bool decode_header(){
            //char header[header_length + 1] = "";
            //std::strncat(header, data_, header_length);//把length长度的字符加到head的上
            //body_length_ = std::atoi(header);//跳过前边的空白字符
            memcpy(&m_header, data(), header_length);
            if (m_header.bodySize > max_body_length)
            {
                std::cout << "body size" << m_header.bodySize << "" << m_header.type << std::endl;
                return false;
            }
            return true;
        }
        
    private:
        char data_[header_length + max_body_length];
        Header m_header;
};
#endif