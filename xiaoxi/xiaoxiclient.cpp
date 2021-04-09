#define BOOST_BIND_GLOBAL_PLACEHOLDERS 
#include "chat_message.hpp"
#include "SerilizationObject.h"
#include "JsonObject.h"
#include "Protocal.pb.h"
//#include "structHeader.h"
#include <boost/asio.hpp>
#include <deque>
#include <iostream>
#include <thread>
#include <cstdlib>
#include <cassert>

#define _GLIBCXX_USE_CXX11_ABI 1
using boost::asio::ip::tcp;
//typedef std::deque<chat_message> chat_message_queue;
using chat_message_queue = std::deque<chat_message>;

class chat_client
{
private:
    boost::asio::io_service &io_service_;
    tcp::socket socket_;
    chat_message read_msg_;
    chat_message_queue write_msgs_;

public:
    chat_client(boost::asio::io_service &io_service,
    tcp::resolver::iterator endpoint_iterator):io_service_(io_service),socket_(io_service){
        do_connect(endpoint_iterator);
    }

    void write(const chat_message &msg){
        io_service_.post(               //生成一个时间在io_service下区跑
            [this, msg]() {
                bool write_in_progress = !write_msgs_.empty();
                write_msgs_.push_back(msg);
                if(!write_in_progress){
                    do_write();
                    std::cout << "write "<<std::endl;
                }
            });
    }
    void close(){
        io_service_.post([this]() { socket_.close(); });
    }
private:
    void do_connect(tcp::resolver::iterator endpoint_iterator){
        std::cout << "connect" << std::endl;
        boost::asio::async_connect(
            socket_, endpoint_iterator, [this](boost::system::error_code ec, tcp::resolver::iterator) {
                if(!ec)
                {
                    do_read_header();
                }
            });
    }
    void do_read_header(){
        boost::asio::async_read(
            socket_, boost::asio::buffer(read_msg_.data(), chat_message::header_length),
            [this](boost::system::error_code ec, std::size_t) {
                if(!ec&&read_msg_.decode_header())
                {
                    do_read_body();
                }
                else
                {
                    socket_.close();
                }
            });
    }
    void do_read_body(){
        boost::asio::async_read(
            socket_,boost::asio::buffer(read_msg_.body(),read_msg_.body_length()),
            [this](boost::system::error_code ec,std::size_t){
                if(!ec){
                    if(read_msg_.type()==MT_ROOM_INFO){
                        //SRoomInfo info;
                        std::string buffer(read_msg_.body(), read_msg_.body() + read_msg_.body_length());
                        PRoomInformation roomInfo;
                        auto ok = roomInfo.ParseFromString(buffer);
                        //if(!ok)
                        //    throw std::runtime_error("not vaild message");
                        if (ok)
                        {
                            std::cout << "client: ";
                            std::cout << roomInfo.name();
                            std::cout << " says ";
                            std::cout << roomInfo.infomation();
                            std::cout << "\n";
                        }
                    }
                    //std::cout.write(read_msg_.body(), read_msg_.body_length());
                    //std::cout << "\n";
                    do_read_header();
                }else{
                    socket_.close();
                }
            }
        );
    }
    
    void do_write(){
        std::cout << "realwrite "<<std::endl;
        boost::asio::async_write(
            socket_,boost::asio::buffer(write_msgs_.front().data(),write_msgs_.front().length()),
            [this](boost::system::error_code ec,std::size_t){
                if(!ec){
                    write_msgs_.pop_front();
                    if(!write_msgs_.empty()){
                        
                        do_write();
                        
                    }
                }else{
                    socket_.close();
                }
            }
        );
    }
};
int main(int argc,char*argv[]){
try{
    GOOGLE_PROTOBUF_VERIFY_VERSION;//检查版本
    if (argc != 3)
    {
        std::cerr << "port...\n";
        return 1;
    }
    boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);
    auto endpoint_iterator = resolver.resolve({argv[1], argv[2]});
    chat_client c(io_service, endpoint_iterator);
    std::thread t(
        [&io_service]() { io_service.run(); });
    char line[chat_message::max_body_length + 1];
    while (std::cin.getline(line,chat_message::max_body_length+1)){//ctrl+d正常退出
        chat_message msg;
        auto type = 0;
        std::string input(line, line + std::strlen(line));//转换成字符串，消息的头部和尾部，都是指针
        std::string output;
        
        if(parseMessage4(input,&type,output)){
            msg.setMessage(type, output.data(), output.size());
            c.write(msg);
            std::cout << "write message for server" << output.size() << std::endl;
        }
    }
    c.close();
    t.join();
}catch (std::exception &e){
    std::cerr << "exception:" << e.what() << "\n";
}
google::protobuf::ShutdownProtobufLibrary();//主动释放proto申请的内存
return 0;
}

