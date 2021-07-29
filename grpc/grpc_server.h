#pragma once

#include "../http2/connection.h"
#include "../http2/stream.h"
#include "../http2/stream_handler.h"
#include "../http2/headers_frame.h"
#include "../http2/data_frame.h"
#include "unary.h"
#include "grpc_stream.h"
#include <vector>

using namespace std;

class GRPCServer : public StreamHandler {
public:
	void addConnection(Connection* conn);
	void removeConnection(Connection* conn);
	void runOnce();
	void registerUnaryMethod(UnaryMethodHandler* handler);
private:
	void newStream(http2::Stream* stream, HeadersFrame* frame);
	void streamData(http2::Stream* stream, DataFrame* frame);
	void rstStream(http2::Stream* stream);
	UnaryMethodHandler* findMethod(char* name);
	GRPCStream* findStream(http2::Stream* h2Stream);

	vector<Connection*> connections;
	vector<UnaryMethodHandler*> unaryMethods;
	vector<GRPCStream*> streams;
};
