#pragma once

#include <stdint.h>
#include <Ethernet.h>
#include "../http2/transport.h"

class EthernetClientTransport : public Transport {
public:
    EthernetClientTransport(EthernetClient client);
    size_t write(uint8_t byte);
    int available();
    int read();
    bool connected();
private:
    EthernetClient client;
};
