#ifndef FND_H
#define FND_H
#include<string>
#include "JsonObject.h"
#include "Protocal.pb.h"
//#define _GLIBCXX_USE_CXX11_ABI 0  #对我来说能用，但出现了很多redefine的警告
struct Header
{
    int bodySize;
    int type;
};
enum MessageType
{
    MT_BIND_NAME = 1,//{"name":"adc"}客户端发给服务器
    MT_CHAT_INFO = 2,//{"information":"what i say"}客户端发给服务器
    MT_ROOM_INFO = 3 //{"name":"abc","informattion":"what i say"}服务器发给客户端
};
//client send
struct BindName
{
    /* data */
    char name[32];
    int nameLen;
};
//client send
struct ChatInformation
{
    /* data */
    char information[256];
    int infoLen;
};
//server send
struct RoomInformation
{
    /* data */
    BindName name;
    ChatInformation chat;
};
bool parseMessage(const std::string &input, int *type, std::string &outbuffer);
bool parseMessage2(const std::string &input, int *type, std::string &outbuffer);
bool parseMessage3(const std::string &input, int *type, std::string &outbuffer);
bool parseMessage4(const std::string &input, int *type, std::string &outbuffer);
#endif
