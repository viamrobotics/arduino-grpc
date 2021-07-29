#include "../utils/utils.h"
#include "grpc_server.h"
#include <algorithm>
#include "../http2/hpack.h"
#include <string.h>
#include <memory>
#include "grpc_stream.h"
#include "unary.h"

void GRPCServer::registerUnaryMethod(UnaryMethodHandler* handler) {
	this->unaryMethods.push_back(handler);
}

void GRPCServer::addConnection(Connection* conn) {
	this->connections.push_back(conn);
	conn->streamHandler = this;
}

void GRPCServer::removeConnection(Connection* conn) {
	this->connections.erase(std::remove(this->connections.begin(), this->connections.end(), conn), this->connections.end());
}

void GRPCServer::runOnce() {
	for (Connection* conn : this->connections) {
		while (conn->transport->available()) {
	        int c = conn->transport->read();
	        if (!conn->pump(c)) {
	            break;
	        }
	    }
	}
}

UnaryMethodHandler* GRPCServer::findMethod(char* name) {
	for (UnaryMethodHandler* handler : this->unaryMethods) {
		if (strcmp(handler->name, name) == 0) {
			return handler;
		}
	}
	return nullptr;
}

void GRPCServer::newStream(http2::Stream* stream, HeadersFrame* frame) {

	shared_ptr<char> method;
	for (HeaderField field : frame->headerFields) {
		if (strcmp(field.name.get(), ":path") == 0) {
			method = field.value;
			break;
		}
	}
	if (method.get() == nullptr) {
		debugPrint("write out an error frame here for bad request");
		return;
	}
	UnaryMethodHandler* handler = findMethod(method.get());
	if (handler == nullptr) {
		debugPrint("write out an error frame here for unimplemented");
		debugPrint(method.get());
		return;	
	}
	this->streams.push_back(new GRPCStream(stream, handler));
}

GRPCStream* GRPCServer::findStream(http2::Stream* h2Stream) {
	for (GRPCStream* stream : this->streams) {
		if (stream->stream == h2Stream) {
			return stream;
		}
	}
	return nullptr;
}

void GRPCServer::streamData(http2::Stream* stream, DataFrame* frame) {
	GRPCStream* grpcStream = findStream(stream);
	if (grpcStream == nullptr) {
		debugPrint("write error frame for stream not found by id");
		return;
	}
	uint8_t* outData = nullptr;
	uint32_t outDataLen = 0;

	// TODO(erd): wait for EOS
	// NOTE(erd): assuming this is an EOS

	uint8_t ret = grpcStream->handler->handler(frame->data+5, frame->dataLength-5, &outData, &outDataLen);

	if (ret != 0) {
		debugPrint("need to send an UNKNOWN error back");
		return;
	}
	grpcStream->writeResponse(outData, outDataLen);
	if (outDataLen != 0) {
		delete[] outData;
	}
}

void GRPCServer::rstStream(http2::Stream* stream) {
	GRPCStream* grpcStream = findStream(stream);
	if (grpcStream == nullptr) {
		debugPrint("write error frame for stream not found by id");
		return;
	}
	this->streams.erase(std::remove(this->streams.begin(), this->streams.end(), grpcStream), this->streams.end());
}
