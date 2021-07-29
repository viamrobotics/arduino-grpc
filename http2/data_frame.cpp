#include "data_frame.h"

uint32_t DataFrame::payloadLength() {
    uint32_t length = this->dataLength;
    if (this->paddingLength != 0) {
        length += this->paddingLength + 1;
    }
    return length;
}

void DataFrame::writePayload(Transport * transport) {
    if (this->paddingLength != 0) {
        transport->write(this->paddingLength);
    }
    for (uint8_t i = 0; i < this->dataLength; i++) {
        transport->write(this->data[i]);
    }
    for (uint8_t i = 0; i < this->paddingLength; i++) {
        transport->write(this->padding[i]);
    }
}
