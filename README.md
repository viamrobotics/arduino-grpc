# gRPC server for Arduino

This library minimally supports a gRPC server that can handle unary methods on top of an Arduino. It has only been tested on an Arduino Due with an Ethernet Shield 2.0 so far.

The server is built in a byte pumped based version of HTTP/2 framing (HTTP/2 semantics are not implemented).

*Note:* This library is experimental and should not yet be used for production use cases.

## Installation

* Copy the `arduino`, `http2`, `grpc`, and `utils` directories either directly into your project or into your sketch `libraries` directory.

## Bugs
* Arduino crashes/corrupts itself after an indeterminate amount of requests
	* Running sanitizers on unix has not revealed anything yet.

## TODO
* Return errors properly
* Return unimplemented gRPC methods properly
* Implement remaining HTTP/2 frames (e.g. CONTINUATION)
* Support streaming use-cases (hard?)
* Address crashes only present on Arduino but not on unix
* Improve performance (avoid single byte transport writes, zero memory copies)

## Protobuf
`UnaryMethodHandler` does not concern itself with what protobuf library is being used but will provide pointers to the raw request data and a pointer to set the response data. The examples are using [nanopb](https://github.com/nanopb/nanopb).

## Examples
* [examples/arduino/server](./examples/arduino/server) - simple arduino server for testing
* [examples/unix](./examples/unix) - simple unix-like server for testing

## Acknowledgements
* Implementation of HTTP/2 HPACK https://pkg.go.dev/golang.org/x/net/http2/hpack
	* Adapted implementation of HPACK and Huffman coding.
