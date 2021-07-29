#include "frame_header.h"

FrameHeader::FrameHeader() {
    this->length = 0;
    this->type = FrameType::DATA;
    this->flags = 0x0;
    this->streamID = 0;    
}

FrameHeader::FrameHeader(FrameType type, uint8_t streamID) {
    this->length = 0;
    this->type = type;
    this->flags = 0x0;
    this->streamID = streamID;    
}
