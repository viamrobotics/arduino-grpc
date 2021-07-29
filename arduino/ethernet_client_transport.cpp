#include "ethernet_client_transport.h"

EthernetClientTransport::EthernetClientTransport(EthernetClient client) {
    this->client = client;
}

size_t EthernetClientTransport::write(uint8_t byte) {
    return this->client.write(byte);
}

int EthernetClientTransport::available() {
    return this->client.available();
}

int EthernetClientTransport::read() {
    return this->client.read();
}

bool EthernetClientTransport::connected() {
    return this->client.connected();
}
