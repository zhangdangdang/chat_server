#include "chat_message.hpp"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <utility>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include <deque>
#include <list>
#include <set>
#include <utility>

//保证兼容性
#if BOOST_VERSION >= 107000
#define GET_IO_SERVICE(s) ((boost::asio::io_context&)(s).get_executor().context())
#else
#define GET_IO_SERVICE(s) ((s).get_io_service())
#endif
 
//boost::asio::io_service& get_io_service(){
 //           return GET_IO_SERVICE(acceptor_);
 //       }
using boost::asio::ip::tcp;
using namespace std;
using chat_message_queue = std::deque<chat_message>;
class chat_session;
using chat_session_ptr = std::shared_ptr<chat_session>;
class chat_room
{
public:
    void join(chat_session_ptr);
    void leave(chat_session_ptr);
    void deliver(const chat_message &msg);

private:
    set<chat_session_ptr> participants_;
    enum
    { 
        max_recent_msgs = 100
    };
    chat_message_queue recent_msgs_;
};
class chat_session : public std::enable_shared_from_this<chat_session>
{

public:
    chat_session(tcp::socket socket, chat_room &room) : socket_(std::move(socket)), room_(room) {}

    void start()
    {
        room_.join(shared_from_this());
        do_read_header();
    }
    void deliver(const chat_message &msg)//96详解12 23：34
    {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress)
        {
            do_write();
        }
    }

private:
    void do_read_header()
    {
        std::cout << "read_header" << std::endl;
        auto self(shared_from_this());
        boost::asio::async_read(
            socket_, boost::asio::buffer(read_msg_.data(), chat_message::header_length),
            [this, self](boost::system::error_code ec, std::size_t) {
                if (!ec && read_msg_.decode_header())
                {
                    do_read_body();
                }
                else
                {
                    room_.leave(shared_from_this());
                }
            });
    }

    void do_read_body()
    {
        std::cout << "read_body" << std::endl;
        auto self(shared_from_this());
            boost::asio::async_read(
                socket_,boost::asio::buffer(read_msg_.body(),read_msg_.body_length()),
                [this,self](boost::system::error_code ec,std::size_t){
                    if(!ec){
                        //room_.deliver(read_msg_);
                        handleMessage();
                        do_read_header();
                    }else{
                        room_.leave(shared_from_this());
                    }
                }
            );
        }
    void handleMessage(){
        if(read_msg_.type()==MT_BIND_NAME){
            const BindName *bind = reinterpret_cast<const BindName *>(read_msg_.body());
            m_name.assign(bind->name, bind->name + bind->nameLen);
        }else if(read_msg_.type()==MT_CHAT_INFO){
            const ChatInformation *chat = reinterpret_cast<const ChatInformation *>(read_msg_.body());
            m_chatInformation.assign(chat->information, chat->information + chat->infoLen);
            auto rinfo = buildRoomInfo();
            chat_message msg;
            msg.setMessage(MT_ROOM_INFO, &rinfo, sizeof(rinfo));
            room_.deliver(msg);
        }else{
                //not valid msg do nothing
        }
    }
    void do_write(){
            auto self(shared_from_this());
            boost::asio::async_write(
                socket_,boost::asio::buffer(write_msgs_.front().data(),
                                            write_msgs_.front().length()),
                [this,self](boost::system::error_code ec,std::size_t){
                    if(!ec){
                        write_msgs_.pop_front();
                        if(!write_msgs_.empty()){
                            do_write();
                            printf("duqu\n");
                        }
                    }else{
                        room_.leave(shared_from_this());
                    }
                }
            );
        }
        tcp::socket socket_;
        chat_room &room_;
        chat_message read_msg_;
        chat_message_queue write_msgs_;
        std::string m_name;
        std::string m_chatInformation;
        RoomInformation buildRoomInfo()const{
            RoomInformation info;
            info.name.nameLen = m_name.size();
            std::memcpy(info.name.name, m_name.data(), m_name.size());
            info.chat.infoLen = m_chatInformation.size();
            std::memcpy(info.chat.information, m_chatInformation.data(), m_chatInformation.size());
            return info;
        }
};

void chat_room::join(chat_session_ptr participant)
        {
            participants_.insert(participant);
            for (const auto &msg : recent_msgs_)
                participant->deliver(msg);
        }
void chat_room::leave(chat_session_ptr participant){
        participants_.erase(participant);
    }
void chat_room::deliver(const chat_message &msg){
        recent_msgs_.push_back(msg);
        while(recent_msgs_.size()>max_recent_msgs)
            recent_msgs_.pop_front();
        for(auto &participant:participants_)
            participant->deliver(msg);
    }



class chat_server{
    public:
        chat_server(boost::asio::io_service &io_service,const tcp::endpoint &endpoint):
        acceptor_(io_service,endpoint),socket_(io_service){
            do_accept();
        }
    private:
    void do_accept(){
        acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
            if(!ec)
            {
                std::make_shared<chat_session>(std::move(socket_), room_)->start();
                std::cout << "accept" << std::endl;
            }
            do_accept();
        });
    }
    tcp::socket socket_;
    tcp::acceptor acceptor_;
    chat_room room_;
};
int main (int argc,char *argv[]){
    try{
        if(argc<2){
            std::cerr << "port\n";
            return 1;
        }
        boost::asio::io_service io_service;
        std::list<chat_server> servers;
        for (int i = 1; i < argc;++i){
            tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
            servers.emplace_back(io_service, endpoint);
        }
        io_service.run();
    }catch(std::exception &e){
        std::cerr << "exception:" << e.what() << "\n";
    }
    return 0;
}