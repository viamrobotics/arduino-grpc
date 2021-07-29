#include "stream.h"
#include "headers_frame.h"
#include "data_frame.h"

namespace http2 {
	void Stream::writeHeaders(HeadersFrame* frame) {
		this->frameWriter->writeFrame(frame);
	}

	void Stream::writeData(DataFrame* frame) {
		this->frameWriter->writeFrame(frame);
	}	
}
