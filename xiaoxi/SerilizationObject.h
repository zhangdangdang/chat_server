#ifndef SER_OBJ
#define SER_OBJ
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
class SBindName
{
private:
    friend class boost::serialization::access;
    template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & m_bindName;//&重载 把内容写入ar中
        //ar<<m_bindname
        //ar>>m_bindname
		
	}
    std::string m_bindName;

public:
    SBindName(std::string name):m_bindName(std::move(name)){}
    SBindName(){}
    const std::string &bindName() const { return m_bindName; }
    
};
class SChatInfo{
 private:   
    friend class boost::serialization::access;
    template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & m_chatInformation;//&重载 把内容写入ar中
        //ar<<m_bindname
        //ar>>m_bindname
		
	}
    std::string m_chatInformation;
public:
    SChatInfo(std::string info):m_chatInformation(std::move(info)){}
    SChatInfo(){}
    const std::string &chatInformation() const { return m_chatInformation; }
};

class SRoomInfo{
 private:   
    friend class boost::serialization::access;
    template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & m_bind;//&重载 把内容写入ar中
        ar & m_chat;
        //ar<<m_bindname
        //ar>>m_bindname
		
	}
    SBindName m_bind;
    SChatInfo m_chat;

public:
    SRoomInfo(std::string name,std::string info):m_bind(std::move(name)),m_chat(std::move(info)){}
    SRoomInfo(){}
    const std::string &name() const { return m_bind.bindName(); }
    const std::string &information() const { return m_chat.chatInformation(); }
};









#endif