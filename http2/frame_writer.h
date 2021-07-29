#pragma once

#include "frame.h"

struct FrameWriter {
	virtual void writeFrame(Frame* frame) = 0;
};
