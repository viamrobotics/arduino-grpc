#include "socket_transport.h"
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

SocketTransport::SocketTransport(tcp::socket* socket) {
    this->socket = socket;
}

size_t SocketTransport::write(uint8_t byte) {
	return boost::asio::write(*(this->socket), boost::asio::buffer(&byte, 1));
}

int SocketTransport::available() {
    return this->socket->available();
}

int SocketTransport::read() {
	char buffer[1];
    boost::asio::read(*(this->socket), boost::asio::buffer(buffer, 1));
    return buffer[0];
}

bool SocketTransport::connected() {
	// TODO(erd): does this matter?
	return true;
}
