#include "../utils/utils.h"
#include "frame_reader.h"
#include "../utils/net.h"
#include "data_frame.h"
#include "headers_frame.h"
#include "priority_frame.h"
#include "reset_stream_frame.h"
#include "settings_frame.h"
#include "push_promise_frame.h"
#include "ping_frame.h"
#include "go_away_frame.h"
#include "window_update_frame.h"

enum FrameState {
    ReadingHeader,
    HeaderRead,
    ReadingPayload,
    Ready
};

Result<bool> FrameReader::pump(uint8_t byte) {
    if (this->readingHeader) {
        Result<bool> pumpResult = this->frameHeaderReader.pump(byte);
        if (!pumpResult.isOk() || !pumpResult.value()) {
            // TODO(erd): DISCARD, do not fail? always?
            if (!pumpResult.isOk()) {
                debugPrint(pumpResult.error());
                debugPrint("ConnectionState::Failed");
            }
            return pumpResult;
        }
        if (!pumpResult.value()) {
            return Result<bool>::ofValue(false);
        }
        this->readingHeader = false;

        FrameHeader header = this->frameHeaderReader.header();

        if (header.length == 0) {
            return Result<bool>::ofValue(true);
        }

        this->readingPayload = true;
        this->framePayload = new uint8_t[header.length];
        this->framePayloadBytesLeft = header.length;
        return Result<bool>::ofValue(false);
    }

    if (this->readingPayload) {
        if (this->framePayloadBytesLeft == 0) {
            return Result<bool>::ofValue(true);
        }
        this->framePayload[this->frameHeaderReader.header().length-this->framePayloadBytesLeft] = byte;
        this->framePayloadBytesLeft--;
        if (this->framePayloadBytesLeft != 0) {
            return Result<bool>::ofValue(false);
        }
    }

    return Result<bool>::ofValue(true);
}

Result<Frame*> FrameReader::frame() {
    if (this->_frame != nullptr) {
        return Result<Frame*>::ofValue(this->_frame);
    }   
    // construct frame
    FrameHeader header = this->frameHeaderReader.header();
    switch (header.type) {
        case DATA: {
            // TODO(erd): complete (handle padding)
            _frame = new DataFrame(header, this->framePayload, header.length);
        }
            break;
        case HEADERS: {
            Result<vector<HeaderField>> headerFieldsResult = decodeHeaderFields(this->framePayload, header.length);
            if (!headerFieldsResult.isOk()) {
                return Result<Frame*>::ofError(headerFieldsResult.error());
            }

            vector<HeaderField> headerFields = headerFieldsResult.value();
            _frame = new HeadersFrame(header, headerFields);
        }
            break;
        case PRIORITY: {
            uint32_t value = readUint32(this->framePayload);
            uint32_t streamDependency = value & 0x7fffffff;
            bool exclusive = value != streamDependency;
            uint8_t weight = this->framePayload[4];
            _frame = new PriorityFrame(
                header,
                exclusive,
                streamDependency,
                weight);
        }
            break;
        case RST_STREAM: {
            uint32_t errorCodeInt = readUint32(this->framePayload);
            ErrorCode errorCode;
            switch (errorCodeInt) {
                case NO_ERROR:
                case PROTOCOL_ERROR:
                case INTERNAL_ERROR:
                case FLOW_CONTROL_ERROR:
                case SETTINGS_TIMEOUT:
                case STREAM_CLOSED:
                case FRAME_SIZE_ERROR:
                case REFUSED_STREAM:
                case CANCEL:
                case COMPRESSION_ERROR:
                case CONNECT_ERROR:
                case ENHANCE_YOUR_CALM:
                case INADEQUATE_SECURITY:
                case HTTP_1_1_REQUIRED:
                    errorCode = static_cast<ErrorCode>(errorCodeInt);
                    break;
                default:
                    debugPrint("unknown error code");
                    errorCode = PROTOCOL_ERROR;
            }
            _frame = new ResetStreamFrame(header, errorCode);
        }
            break;
        case SETTINGS: {
            // TODO(erd): move check higher to avoid alloc
            if (header.length % 6 != 0) {
                debugPrint("SETTINGS payload must be divisible by 6");
                // TODO(erd): fail stream
                debugPrint("ConnectionState::Failed");
                break;
            }
            bool isAck = header.flags & SETTINGS_FLAG_ACK;
            if (isAck) {
                this->_frame = SettingsFrame::makeAck(header);
            } else {
                std::vector<Setting> settings;
                for (int i = 0; i < header.length; i+=6) {
                    uint16_t identifier = (this->framePayload[i] << 8) + this->framePayload[i+1];
                    uint32_t value = (this->framePayload[i+2] << 24) + (this->framePayload[i+3] << 16)
                        + (this->framePayload[i+4] << 8) + this->framePayload[i+5];
                    bool isValid = true;
                    switch (identifier) {
                        case SETTINGS_HEADER_TABLE_SIZE:
                        case SETTINGS_ENABLE_PUSH:
                        case SETTINGS_MAX_CONCURRENT_STREAMS:
                        case SETTINGS_INITIAL_WINDOW_SIZE:
                        case SETTINGS_MAX_FRAME_SIZE:
                            break;
                        default:
                            isValid = false;
                            break;
                    }
                    if (isValid) {
                        settings.push_back(Setting{.parameter = static_cast<SettingParameter>(identifier), .value = value});
                    }
                }
                SettingsFrame* frame = new SettingsFrame(header);
                frame->settings = settings;
                this->_frame = frame;
            }
        }
            break;
        case PUSH_PROMISE: {
            return Result<Frame*>::ofError("do not know how to make PUSH_PROMISE");
        }
        case PING: {
            bool isAck = header.flags & PING_FLAG_ACK;
            if (isAck) {
                this->_frame = PingFrame::makeAck(header, this->framePayload);
            } else {
                this->_frame = new PingFrame(header, this->framePayload);
            }
            break;
        }
        case GOAWAY: {
            return Result<Frame*>::ofError("do not know how to make GOAWAY");
        }
        case WINDOW_UPDATE: {
            // TODO(erd): ignore upper bit (it's reserved)
            uint32_t increment = readUint32(this->framePayload);
            this->_frame = new WindowUpdateFrame(header, increment);
        }
        break;
        case CONTINUATION: {
            return Result<Frame*>::ofError("do not know how to make CONTINUATION");
        }
        default:
            return Result<Frame*>::ofError("unknown frame type");
    }
    return Result<Frame*>::ofValue(this->_frame);
}
