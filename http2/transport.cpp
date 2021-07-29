#include "transport.h"

size_t Transport::write(uint32_t value) {
    size_t written = 0;
    written += this->write(static_cast<uint8_t>((value & 0xff000000) >> 24));
    written += this->write(static_cast<uint8_t>((value & 0xff0000) >> 16));
    written += this->write(static_cast<uint8_t>((value & 0xff00) >> 8));
    written += this->write(static_cast<uint8_t>(value & 0xff));
    return written;
}
