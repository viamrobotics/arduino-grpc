#pragma once

#include "stream.h"
#include "headers_frame.h"
#include "data_frame.h"

class StreamHandler {
public:
	virtual void newStream(http2::Stream* stream, HeadersFrame* frame) = 0;
	virtual void streamData(http2::Stream* stream, DataFrame* frame) = 0;
	virtual void rstStream(http2::Stream* stream) = 0;
};
