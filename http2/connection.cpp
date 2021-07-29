#include "../utils/utils.h"
#include "connection.h"
#include "reset_stream_frame.h"
#include "settings_frame.h"
#include "headers_frame.h"
#include "data_frame.h"
#include "ping_frame.h"
#include <algorithm>

// TODO(erd): return type
void Connection::writeFrame(Frame* frame) {
    frame->write(this->transport);
}

// https://datatracker.ietf.org/doc/html/rfc7540#section-3.5
// PRI *HTTP/2.0\r\n\r\nSM\r\n\r\n
// This is the first data we see since we expect an h2c connection (https://datatracker.ietf.org/doc/html/rfc7540#section-3.4)
#define CONNECTION_PREFACE_SIZE 24
char CONNECTION_PREFACE[CONNECTION_PREFACE_SIZE] = {
    0x50, 0x52, 0x49, 0x20, 0x2a, 0x20, 0x48, 0x54, 0x54, 0x50, 0x2f, 0x32, 0x2e, 0x30, 0x0d, 0x0a, 0x0d, 0x0a, 0x53, 0x4d, 0x0d, 0x0a, 0x0d, 0x0a
};

bool Connection::pump(uint8_t byte) {
    switch (this->connState) {
        case Started:
            if (CONNECTION_PREFACE[this->prefacePos] == byte) {
                this->prefacePos++;
                if (this->prefacePos == CONNECTION_PREFACE_SIZE) {
                    this->connState = ConnectionState::Connected;

                    // Send first SETTINGS frame with hpack "disabled"
                    // TODO(erd): wait for ack when?
                    SettingsFrame settingsFrame;
                    settingsFrame.settings.push_back(Setting{.parameter = SETTINGS_HEADER_TABLE_SIZE, .value = 0});
                    this->writeFrame(&settingsFrame);
                }
            } else {
                debugPrint("invalid preface at pos: ");
                debugPrint(this->prefacePos);
                debugPrint("\n");
                debugPrint("ConnectionState::Failed");
                this->connState = ConnectionState::Failed;
                return false;
            }
            break;
        case Connected: {
            Result<bool> pumpResult = this->currFrameReader->pump(byte);
            if (!pumpResult.isOk() || !pumpResult.value()) {
                if (!pumpResult.isOk()) {
                    // TODO(erd): DISCARD, do not fail? always?
                    debugPrint(pumpResult.error());
                    debugPrint("ConnectionState::Failed");
                    this->connState = ConnectionState::Failed;
                }
                return false;
            }
            Result<Frame*> frameResult = this->currFrameReader->frame();
            if (!frameResult.isOk()) {
                // TODO(erd): DISCARD, do not fail? always?
                debugPrint(frameResult.error());
                debugPrint("ConnectionState::Failed");
                this->connState = ConnectionState::Failed;
                return false;
            }
            Frame* currFrame = frameResult.value();

            if (!this->prefaceSettingsAckd && currFrame->header.type != SETTINGS) {
                debugPrint("dropping frame and sending RST_STREAM since no ack yet");
                ResetStreamFrame rstFrame = ResetStreamFrame(currFrame->header.streamID);
                this->writeFrame(&rstFrame);
            } else {
                switch (currFrame->header.type) {
                    case DATA: {
                        http2::Stream* stream = this->getStream(currFrame->header.streamID);
                        DataFrame* dataFrame = static_cast<DataFrame*>(currFrame);
                        this->streamHandler->streamData(stream, dataFrame);
                        if (dataFrame->header.flags & DATA_FLAG_END_STREAM) {
                            this->streamHandler->rstStream(stream);
                            this->deallocateStream(currFrame->header.streamID);
                        }
                    }
                        break;
                    case HEADERS: {
                        http2::Stream* stream = this->allocateStream(currFrame->header.streamID);
                        this->streamHandler->newStream(stream, static_cast<HeadersFrame*>(currFrame));
                    }
                        break;
                    case PRIORITY:
                        break;
                    case RST_STREAM: {
                        http2::Stream* stream = this->getStream(currFrame->header.streamID);
                        this->streamHandler->rstStream(stream);
                        this->deallocateStream(currFrame->header.streamID);
                    }
                        break;
                    case SETTINGS: {
                        bool isAck = currFrame->header.flags & SETTINGS_FLAG_ACK;
                        if (isAck) {
                            if (!this->prefaceSettingsAckd && currFrame->header.streamID == 0) {
                                this->prefaceSettingsAckd = true;
                            }
                        } else {
                            // TODO(erd): process settings?
                            SettingsFrame ackFrame = SettingsFrame::ack(currFrame->header.streamID);
                            this->writeFrame(&ackFrame);
                        }
                    }
                        break;
                    case PUSH_PROMISE:
                        break;
                    case PING: {
                        bool isAck = currFrame->header.flags & PING_FLAG_ACK;
                        if (!isAck) {
                            PingFrame ackFrame = PingFrame::ack(static_cast<PingFrame*>(currFrame)->opaque);
                            this->writeFrame(&ackFrame);
                        }
                    }
                        break;
                    case GOAWAY:
                        break;
                    case WINDOW_UPDATE:
                        break;
                    case CONTINUATION:
                        break;
                }
            }
            this->resetFrame();
        }
            break;
        case Disconnected:
            debugPrint("bytes available in disconnected state");
            break;
        case Failed:
            debugPrint("bytes available in failed state");
            char tmp[16];
            sprintf(tmp, "%02x", byte);
            debugPrint(tmp);
            break;
    }
    return true;
}

void Connection::resetFrame() {
    if (this->currFrameReader == nullptr) {
        return;
    }
    delete this->currFrameReader;
    this->currFrameReader = new FrameReader();
}

// TODO(erd): expire old streams?
// TODO(erd): stream limit
http2::Stream* Connection::allocateStream(uint32_t streamID) {
    http2::Stream* stream = new http2::Stream(this, streamID);
    this->streams.push_back(stream);
    return stream;
}

http2::Stream* Connection::getStream(uint32_t streamID) {
    for (http2::Stream* candidate : this->streams) {
        if (candidate->id == streamID) {
            return candidate;
        }
    }
    return nullptr;
}

void Connection::deallocateStream(uint32_t streamID) {
    http2::Stream* stream = this->getStream(streamID);
    if (stream == nullptr) {
        return;
    }
    this->streams.erase(std::remove(this->streams.begin(), this->streams.end(), stream), this->streams.end());    
    delete stream;
}

