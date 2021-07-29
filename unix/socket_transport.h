#pragma once

#include <stdint.h>
#include <boost/asio.hpp>
#include <http2/transport.h>

using boost::asio::ip::tcp;

class SocketTransport : public Transport {
public:
    SocketTransport(tcp::socket* socket);
    size_t write(uint8_t byte);
    int available();
    int read();
    bool connected();
private:
    tcp::socket* socket;
};
