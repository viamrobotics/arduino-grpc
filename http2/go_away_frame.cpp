#include "go_away_frame.h"

uint32_t GoAwayFrame::payloadLength() {
    return 8 + this->debugDataLength;
}

void GoAwayFrame::writePayload(Transport * transport) {
    transport->write(this->lastStreamID);
    transport->write(static_cast<uint32_t>(this->errorCode));
    for (uint8_t i = 0; i < this->debugDataLength; i++) {
        transport->write(this->debugData[i]);
    }
}
