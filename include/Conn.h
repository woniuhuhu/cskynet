#pragma once
using namespace std;
class Conn {   //消息类型
    public:
        enum TYPE {
            LISTEN = 1,
            CLIENT = 2,
        };
        uint8_t type;
        int fd;
        uint32_t serviceId;
};