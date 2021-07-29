#include "../utils/utils.h"
#include "headers_frame.h"

uint32_t HeadersFrame::payloadLength() {
    return encodeHeaderFieldsLen(this->headerFields);;
}

void HeadersFrame::writePayload(Transport * transport) {
    uint32_t encodedLen = 0;
    uint8_t* encoded = encodeHeaderFields(this->headerFields, encodedLen);

    for (uint32_t i = 0; i < encodedLen; i++) {
        transport->write(encoded[i]);
    }
}
