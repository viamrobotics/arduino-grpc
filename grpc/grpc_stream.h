#pragma once

#include "../http2/stream.h"
#include "unary.h"

class GRPCStream {
public:
	GRPCStream(http2::Stream* stream, UnaryMethodHandler* handler)
		: stream(stream), handler(handler) {}
	http2::Stream* stream;
	UnaryMethodHandler* handler;
	void writeResponse(uint8_t* data, uint32_t dataLen);
};
