#pragma once

#include "frame_writer.h"
#include "headers_frame.h"
#include "data_frame.h"

namespace http2 {
	struct Stream {
		Stream(FrameWriter* frameWriter, uint32_t id) : frameWriter(frameWriter), id(id) {}
		FrameWriter* frameWriter = nullptr;
		uint32_t id = 0;
		void writeHeaders(HeadersFrame* frame);
		void writeData(DataFrame* frame);
	};	
}
