#include "grpc_stream.h"
#include "../http2/hpack.h"
#include "../http2/headers_frame.h"
#include "../utils/net.h"

void GRPCStream::writeResponse(uint8_t* data, uint32_t dataLen) {
	vector<HeaderField> headerFields;
	headerFields.push_back(HeaderField(":status", "200"));
	headerFields.push_back(HeaderField("content-type", "application/grpc+proto"));
	HeadersFrame header = HeadersFrame(this->stream->id, headerFields);
	header.header.flags |= HEADERS_FLAG_END_HEADERS;

	this->stream->writeHeaders(&header);

	uint32_t transportDataLen = 1 + 4 + dataLen;
	uint8_t* transportData = new uint8_t[transportDataLen];
	transportData[0] = 0;
	writeUint32(dataLen, transportData+1);
	memcpy(transportData+5, data, dataLen);

	DataFrame dataFrame = DataFrame(this->stream->id, transportData, transportDataLen);
	this->stream->writeData(&dataFrame);
	delete[] transportData;

	vector<HeaderField> trailerFields;
	trailerFields.push_back(HeaderField("grpc-status", "0"));
	HeadersFrame trailer = HeadersFrame(this->stream->id, trailerFields);
	trailer.header.flags |= HEADERS_FLAG_END_HEADERS;
	trailer.header.flags |= HEADERS_FLAG_END_STREAM;
	this->stream->writeHeaders(&trailer);
}

void GRPCStream::writeResponseError(StatusCode code) {
	vector<HeaderField> headerFields;
	headerFields.push_back(HeaderField(":status", "200"));
	headerFields.push_back(HeaderField("content-type", "application/grpc+proto"));
	HeadersFrame header = HeadersFrame(this->stream->id, headerFields);
	header.header.flags |= HEADERS_FLAG_END_HEADERS;

	this->stream->writeHeaders(&header);

	char codeBuf[16];
    sprintf(codeBuf, "%d", static_cast<uint8_t>(code));
	vector<HeaderField> trailerFields;
	trailerFields.push_back(HeaderField("grpc-status", codeBuf));
	HeadersFrame trailer = HeadersFrame(this->stream->id, trailerFields);
	trailer.header.flags |= HEADERS_FLAG_END_HEADERS;
	trailer.header.flags |= HEADERS_FLAG_END_STREAM;
	this->stream->writeHeaders(&trailer);
}
