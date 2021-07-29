/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.6-dev */

#ifndef PB_PROTO_RPC_EXAMPLES_ECHO_V1_SIMPLE_PB_H_INCLUDED
#define PB_PROTO_RPC_EXAMPLES_ECHO_V1_SIMPLE_PB_H_INCLUDED
#include "pb.h"

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _proto_rpc_examples_echo_v1_EchoRequest { 
    char message[16]; 
} proto_rpc_examples_echo_v1_EchoRequest;

typedef struct _proto_rpc_examples_echo_v1_EchoResponse { 
    char message[16]; 
} proto_rpc_examples_echo_v1_EchoResponse;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define proto_rpc_examples_echo_v1_EchoRequest_init_default {""}
#define proto_rpc_examples_echo_v1_EchoResponse_init_default {""}
#define proto_rpc_examples_echo_v1_EchoRequest_init_zero {""}
#define proto_rpc_examples_echo_v1_EchoResponse_init_zero {""}

/* Field tags (for use in manual encoding/decoding) */
#define proto_rpc_examples_echo_v1_EchoRequest_message_tag 1
#define proto_rpc_examples_echo_v1_EchoResponse_message_tag 1

/* Struct field encoding specification for nanopb */
#define proto_rpc_examples_echo_v1_EchoRequest_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, STRING,   message,           1)
#define proto_rpc_examples_echo_v1_EchoRequest_CALLBACK NULL
#define proto_rpc_examples_echo_v1_EchoRequest_DEFAULT NULL

#define proto_rpc_examples_echo_v1_EchoResponse_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, STRING,   message,           1)
#define proto_rpc_examples_echo_v1_EchoResponse_CALLBACK NULL
#define proto_rpc_examples_echo_v1_EchoResponse_DEFAULT NULL

extern const pb_msgdesc_t proto_rpc_examples_echo_v1_EchoRequest_msg;
extern const pb_msgdesc_t proto_rpc_examples_echo_v1_EchoResponse_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define proto_rpc_examples_echo_v1_EchoRequest_fields &proto_rpc_examples_echo_v1_EchoRequest_msg
#define proto_rpc_examples_echo_v1_EchoResponse_fields &proto_rpc_examples_echo_v1_EchoResponse_msg

/* Maximum encoded size of messages (where known) */
#define proto_rpc_examples_echo_v1_EchoRequest_size 17
#define proto_rpc_examples_echo_v1_EchoResponse_size 17

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
