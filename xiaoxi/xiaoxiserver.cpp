#define BOOST_BIND_GLOBAL_PLACEHOLDERS 
#include "chat_message.hpp"
#include "SerilizationObject.h"
#include "Protocal.pb.h"
#include <iostream>
#include <cstdlib>
//#include <utility>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include <deque>
#include <list>
#include <set>
#include "JsonObject.h"
#include <chrono>
#include <mutex>
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>//把log信息放到文件中
#include <boost/bind/bind.hpp>
#include <future>
#include <functional>

//#include <boost/bind/bind.hpp>
using namespace boost::placeholders;

//保证兼容性
#if BOOST_VERSION >= 107000
#define GET_IO_SERVICE(s) ((boost::asio::io_context&)(s).get_executor().context())
#else
#define GET_IO_SERVICE(s) ((s).get_io_service())
#endif
 
//boost::asio::io_service& get_io_service(){
 //           return GET_IO_SERVICE(acceptor_);
 //       }
 //stread_clock
std::chrono::system_clock::time_point base;//基础时间钟表时间
using boost::asio::ip::tcp;
using namespace std;
using chat_message_queue = std::deque<chat_message>;
class chat_session;
using chat_session_ptr = std::shared_ptr<chat_session>;
class chat_room
{
public:
    chat_room(boost::asio::io_service& io_service):m_strand(io_service){}
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
    boost::asio::io_service::strand m_strand;//使用asio函数来保护变量
    //std::mutex m_mutex;
};
class chat_session : public std::enable_shared_from_this<chat_session>
{

public:

    chat_session(tcp::socket socket, chat_room &room) : socket_(std::move(socket)), room_(room),m_strand(GET_IO_SERVICE(socket)) {}

    void start()
    {
        room_.join(shared_from_this());
        do_read_header();
    }
    void deliver(const chat_message &msg)//96详解12 23：34
    {
        m_strand.post([this, msg] {
            bool write_in_progress = !write_msgs_.empty();
            //std::lock_guard<std::mutex> lock(m_mutex);
            write_msgs_.push_back(msg); //deque线程不安全
            if (!write_in_progress)
            {
                do_write();
            }
        });
    }

private:
    boost::asio::io_service::strand  m_strand;
    void do_read_header()
    {
        std::cout << "read_header" << std::endl;
        auto self(shared_from_this());
        boost::asio::async_read(
            socket_, boost::asio::buffer(read_msg_.data(), chat_message::header_length),
            m_strand.wrap(
            [this, self](boost::system::error_code ec, std::size_t) {
                if (!ec && read_msg_.decode_header())
                {
                    do_read_body();
                }
                else
                {
                    room_.leave(shared_from_this());
                }
            }));
    }

    void do_read_body()
    {
        std::cout << "read_body" << std::endl;
        auto self(shared_from_this());
            boost::asio::async_read(
                socket_,boost::asio::buffer(read_msg_.body(),read_msg_.body_length()),
                m_strand.wrap(
                [this,self](boost::system::error_code ec,std::size_t){
                    if(!ec){
                        //room_.deliver(read_msg_);
                        handleMessage();
                        do_read_header();
                    }else{
                        room_.leave(shared_from_this());
                    }
                }
            ));
        }
    template<typename T>
        T toObject(){
        T obj;
        std::stringstream ss(std::string(read_msg_.body(), read_msg_.body()+read_msg_.body_length()));
            boost::archive::text_iarchive oa(ss);
            oa &obj;
            return obj;
    }
    bool fillProtobuf(::google::protobuf::Message* msg)
    {   
        std::string ss(read_msg_.body(), read_msg_.body() + read_msg_.body_length());
        auto ok = msg->ParseFromString(ss);
        return ok;
    }
    void handleMessage(){
        auto n = std::chrono::system_clock::now() - base;
        std::cout << "time  " << std::chrono::duration_cast<std::chrono::milliseconds>(n).count() << std::endl;
        std::cout << "im in " << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));//延时一毫秒
        if (read_msg_.type() == MT_BIND_NAME)
        {
            PBindName bindName;
            //std::string buffer(std::string(read_msg_.body(), read_msg_.body() + read_msg_.body_length()));
            if(fillProtobuf(&bindName))
            m_name = bindName.name();
            std::cout << m_name << std::endl;
        }
        else if (read_msg_.type() == MT_CHAT_INFO)
        {
            
            std::cout << "handleMessage" << std::endl;
            PChat chat;
            if(!fillProtobuf(&chat))//可以做一些日志 调试方便
                return;
            m_chatInformation = chat.information();
            std::cout << m_chatInformation << std::endl;
            auto rinfo = buildRoomInfo();
            chat_message msg;
            //msg.setMessage(MT_ROOM_INFO, &rinfo, sizeof(rinfo));
            msg.setMessage(MT_ROOM_INFO, rinfo);

            room_.deliver(msg);
            std::cout << "hand deliver" << std::endl;
        }
        else
        {
            //not valid msg do nothing
        }
    }
    void do_write(){
            auto self(shared_from_this());
            boost::asio::async_write(
                socket_,boost::asio::buffer(write_msgs_.front().data(),
                                            write_msgs_.front().length()),
                m_strand.wrap(
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
            ));
        }
        tcp::socket socket_;
        chat_room &room_;
        chat_message read_msg_;
        chat_message_queue write_msgs_;
        std::string m_name;
        std::string m_chatInformation;
        std::string buildRoomInfo() const{
            PRoomInformation roomInfo;
            roomInfo.set_name(m_name);
            roomInfo.set_infomation(m_chatInformation);
            std::string out;
            auto ok = roomInfo.SerializeToString(&out);
            assert(ok);
            return out;
        }
};
void chat_room::join(chat_session_ptr participant)
{
    //std::lock_guard<std::mutex> lock(m_mutex);
    m_strand.post([this, participant] {
        participants_.insert(participant);
        for (const auto &msg : recent_msgs_)
            participant->deliver(msg); });
}
void chat_room::leave(chat_session_ptr participant){
    //std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "leave" << std::endl;
    m_strand.post([this, participant] {//post异步执行放到队列里先进先出，mutex竞争性
         participants_.erase(participant);
         });
    
    }
void chat_room::deliver(const chat_message &msg){
    //std::lock_guard<std::mutex> lock(m_mutex);
    std::cerr << "room_deliver\n";
    m_strand.post([this, msg] {
         recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs)
        recent_msgs_.pop_front();
    for (auto &participant : participants_)
        participant->deliver(msg);
         });
    
    }

class chat_server{
    public:
        chat_server(boost::asio::io_service &io_service,const tcp::endpoint &endpoint):
        acceptor_(io_service,endpoint),socket_(io_service),room_(io_service){
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
static std::function<void()> safeQuit;
void signalHandler(int sig){
    BOOST_LOG_TRIVIAL(info)<<"handle system signal"<<sig;
    if(safeQuit){
        safeQuit();
        safeQuit = nullptr;
    }
}
void init(){
    boost::log::add_file_log("log.log");//an tuichu caishudao wenjian
    boost::log::core::get()->set_filter(boost::log::trivial::severity>=boost::log::trivial::info);
}
int main (int argc,char *argv[]){
    try{
        GOOGLE_PROTOBUF_VERIFY_VERSION;//检查protobuf版本
        init();
        BOOST_LOG_TRIVIAL(trace)<<"A trace safe";
        BOOST_LOG_TRIVIAL(debug)<<"A debug safe";
        BOOST_LOG_TRIVIAL(info)<<"A info safe";
        BOOST_LOG_TRIVIAL(warning)<<"A warning ";
        BOOST_LOG_TRIVIAL(error)<<"A error ";
        BOOST_LOG_TRIVIAL(fatal)<<"A fatal ";
        std::cout << "argc"<<argc << std::endl;
        
        if (argc < 2)
        {
            std::cerr << "port\n";
            return 1;
        }
        base = std::chrono::system_clock::now();
        
        boost::asio::io_service io_service;
        safeQuit=[&io_service]{io_service.stop();};
        signal(SIGINT,signalHandler);
        std::list<chat_server> servers;
        for (int i = 1; i < argc;++i){
            tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
            servers.emplace_back(io_service, endpoint);
        }
        std::vector<std::thread> threadGroup;
        for (int i = 0; i < 5;++i){
            threadGroup.emplace_back([&io_service, i] { 
                BOOST_LOG_TRIVIAL(info)<<i<<"name is"<<std::this_thread::get_id()<<std::endl;
                io_service.run(); });
        }
       BOOST_LOG_TRIVIAL(info)<<"main is"<<std::this_thread::get_id()<<std::endl;
        io_service.run();
        for(auto&v:threadGroup)
            v.join();
    }
    catch (std::exception &e)
    {
        BOOST_LOG_TRIVIAL(error)<<"exception "<< e.what() ;
        std::cerr << "exception:" << e.what() << "\n";
    }
    BOOST_LOG_TRIVIAL(info)<<"safe release all resource";
    google::protobuf::ShutdownProtobufLibrary();//主动释放proto申请的内存 内存会自动释放，但是可能会被查内存泄漏
    return 0;
}