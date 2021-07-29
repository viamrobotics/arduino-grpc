#include "src/utils/utils.h"
#include <Ethernet.h>
#include <vector>

#include "src/gen/pb_encode.h"
#include "src/gen/pb_decode.h"
#include "src/gen/simple.pb.h"
#include "src/http2/connection.h"
#include "src/http2/reset_stream_frame.h"
#include "src/http2/settings_frame.h"
#include "src/grpc/grpc_server.h"
#include "src/arduino/ethernet_client_transport.h"

using namespace std;

#define ARDUINO

byte MAC_ADDRESS[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
uint16_t PORT = 8080;

proto_rpc_examples_echo_v1_EchoResponse dummyResponse();
bool encodeResponse(proto_rpc_examples_echo_v1_EchoResponse req, uint8_t** out, size_t* outLen);
bool decodeRequest(proto_rpc_examples_echo_v1_EchoRequest *resp, uint8_t* in, size_t inLen);

EthernetServer server(PORT);
GRPCServer grpcServer = GRPCServer();
vector<Connection*> connections;

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

void setup() {
    Serial.begin(9600);
    Ethernet.begin(MAC_ADDRESS);

    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    debugPrint("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
    }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
        debugPrint("Ethernet cable is not connected.");
    }

    // start the server
    server.begin();
    debugPrint("server is at ");
    debugPrint(Ethernet.localIP());
    debugPrint(":");
    debugPrint(PORT);

    UnaryMethodHandler* echoHandler = new UnaryMethodHandler();
    echoHandler->name = "/proto.rpc.examples.echo.v1.EchoService/Echo";
    echoHandler->handler = echo;
    grpcServer.registerUnaryMethod(echoHandler);
}

void loop() {
    EthernetClient client = server.accept();
    for (auto it = connections.begin(); it != connections.end();) {
        Connection* conn = *it;
        if (conn->transport->connected()) {
            break;
        }
        grpcServer.removeConnection(conn);
        delete conn->transport;
        delete conn;
        it = connections.erase(it);
    }
    if (client) {
        debugPrint("new client");
        // TODO(erd): destructor for transport?
        Connection* conn = new Connection(new EthernetClientTransport(client));
        connections.push_back(conn);
        grpcServer.addConnection(conn);
    } 
    grpcServer.runOnce();
    delay(1);
    return;
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
