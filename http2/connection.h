#pragma once

#include <stdint.h>
#include <vector>
#include "frame.h"
#include "frame_reader.h"
#include "frame_writer.h"
#include "transport.h"
#include "stream.h"
#include "stream_handler.h"

using namespace std;

enum ConnectionState {
    Started,
    Connected,
    Disconnected,
    Failed
};

class Connection : FrameWriter {
public:
    ~Connection() {
        if (this->currFrameReader == nullptr) {
            return;
        }
        delete this->currFrameReader;
    }
    Connection(Transport* transport) {
        this->transport = transport;
        this->currFrameReader = new FrameReader();
        this->prefaceSettingsAckd = false;
    }
    void writeFrame(Frame* frame);
    bool pump(uint8_t byte);

// private:
    void resetFrame();
    Transport* transport;
    uint8_t prefacePos = 0;
    ConnectionState connState = ConnectionState::Started;
    FrameReader* currFrameReader;
    bool prefaceSettingsAckd;

    StreamHandler* streamHandler;
    vector<http2::Stream*> streams;
    http2::Stream* allocateStream(uint32_t streamID);
    void deallocateStream(uint32_t streamID);
    http2::Stream* getStream(uint32_t streamID);
};

