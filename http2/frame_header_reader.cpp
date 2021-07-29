#include "../utils/utils.h"
#include "frame_header_reader.h"

#define FRAME_HEADER_SIZE 9

FrameHeaderReader::FrameHeaderReader() {
    this->_header = FrameHeader();
    this->pos = 0;
}

const char * UNKNOWN_FRAME_TYPE = "unknown frame type";

Result<bool> FrameHeaderReader::pump(uint8_t byte) {
    switch (this->pos) {
        // Length
        case 0:
        case 1:
        case 2:
            this->_header.length |= (byte << (8 * (2 - this->pos)));
            break;
        // Type
        case 3:
            switch (byte) {
                case FrameType::DATA:
                case FrameType::HEADERS:
                case FrameType::PRIORITY:
                case FrameType::RST_STREAM:
                case FrameType::SETTINGS:
                case FrameType::PUSH_PROMISE:
                case FrameType::PING:
                case FrameType::GOAWAY:
                case FrameType::WINDOW_UPDATE:
                    this->_header.type = static_cast<FrameType>(byte);
                    break;
                default:
                    debugPrint("unknown frame type: ");
                    // 100000
                    debugPrint(byte);
                    debugPrint(this->_header.length);
                    // TODO(erd): DISCARD, do not fail
                    return Result<bool>::ofError(UNKNOWN_FRAME_TYPE);
            }
            break;
        // Flags
        case 4:
            this->_header.flags = byte;
            break;
        // Stream ID
        case 5:
        case 6:
        case 7:
        case 8:
            this->_header.streamID |= (byte << (8 * (8 - this->pos)));
            this->_header.streamID &= 0x7fffffff;
            break;
    }
    this->pos++;
    return Result<bool>::ofValue(this->pos == FRAME_HEADER_SIZE);
}

// TODO(erd): exception if not read or failed.
FrameHeader FrameHeaderReader::header() {
    return this->_header;
}
