#include <vector>
#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>

#include <utils/utils.h>
#include <examples/gen/pb_encode.h>
#include <examples/gen/pb_decode.h>
#include <examples/gen/simple.pb.h>
#include <http2/connection.h>
#include <http2/reset_stream_frame.h>
#include <http2/settings_frame.h>
#include <grpc/grpc_server.h>
#include <unix/socket_transport.h>

using boost::asio::ip::tcp;

proto_rpc_examples_echo_v1_EchoResponse dummyResponse();
bool encodeResponse(proto_rpc_examples_echo_v1_EchoResponse req, uint8_t** out, size_t* outLen);
bool decodeRequest(proto_rpc_examples_echo_v1_EchoRequest *resp, uint8_t* in, size_t inLen);

uint8_t echo(uint8_t* data, uint32_t dataLen, uint8_t** response, uint32_t* responseLen) {
    uint8_t* buffer;
    size_t outLen;
    
    // TODO(erd): decode request

    proto_rpc_examples_echo_v1_EchoResponse message = dummyResponse();
    if (!encodeResponse(message, &buffer, &outLen)) {
        debugPrint("encoding failed");
        return 1;
    }

    *response = buffer;
    *responseLen = outLen;
    return 0;
}

int main() {
    GRPCServer grpcServer = GRPCServer();

    UnaryMethodHandler* echoHandler = new UnaryMethodHandler();
    echoHandler->name = "/proto.rpc.examples.echo.v1.EchoService/Echo";
    echoHandler->handler = echo;
    grpcServer.registerUnaryMethod(echoHandler);

     try {
        boost::asio::io_context io_context;

        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));

        for (;;) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            debugPrint("new socket");
            Connection* conn = new Connection(new SocketTransport(&socket));
            grpcServer.addConnection(conn);

            // TODO(erd): when to stop running?
            while (true) {
                grpcServer.runOnce();
            }

            grpcServer.removeConnection(conn);
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

proto_rpc_examples_echo_v1_EchoResponse dummyResponse() {
    proto_rpc_examples_echo_v1_EchoResponse message = proto_rpc_examples_echo_v1_EchoResponse_init_zero;
    strncpy(message.message, "foo", sizeof(message.message));   
    return message;
}

bool encodeResponse(proto_rpc_examples_echo_v1_EchoResponse resp, uint8_t** out, size_t* outLen) {
    size_t bufSize = 1024*sizeof(uint8_t);
    *out = new uint8_t[bufSize];
    pb_ostream_t stream = pb_ostream_from_buffer(*out, bufSize);
    bool status = pb_encode(&stream, proto_rpc_examples_echo_v1_EchoResponse_fields, &resp);
    *outLen = stream.bytes_written;
    return status;
}

bool decodeRequest(proto_rpc_examples_echo_v1_EchoRequest *resp, uint8_t* in, size_t inLen) {
    pb_istream_t stream = pb_istream_from_buffer(in, inLen);
    return pb_decode(&stream, proto_rpc_examples_echo_v1_EchoRequest_fields, resp);
}
