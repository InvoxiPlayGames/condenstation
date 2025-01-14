#ifndef STEAMCLIENT_PROTOBUF_VTABLE_H_
#define STEAMCLIENT_PROTOBUF_VTABLE_H_
#include <stdbool.h>
#include <std_string.h>

typedef void *(*ProtobufDestructor_t)(void *protobuf);
typedef std_basic_string *(*ProtobufGetTypeName_t)(std_basic_string *out_string);
typedef void *(*ProtobufNew_t)(void);
typedef void (*ProtobufClear_t)(void *protobuf);
typedef bool (*ProtobufIsInitialized_t)();
typedef std_basic_string *(*ProtobufInitializationErrorString_t)(std_basic_string *out_string);
typedef void (*ProtobufMergeFrom_t)(void *protobuf, void *from);
typedef bool (*ProtobufMergePartialFromCodedStream_t)(void *protobuf, void *codedInputStream);
typedef int (*ProtobufByteSize_t)(void *protobuf);
typedef void (*ProtobufSerializeWithCachedSizes_t)(void *protobuf, void *codedOutputStream);
typedef void (*ProtobufSerializeToArray_t)(void *protobuf, void *outputArray);
typedef int (*ProtobufGetCachedSize_t)(void *protobuf);

typedef struct _sc_protobuf_vtable_t {
    ProtobufDestructor_t Destroy;
    ProtobufDestructor_t Destroy2;
    ProtobufGetTypeName_t GetTypeName;
    ProtobufNew_t New;
    ProtobufClear_t Clear;
    ProtobufIsInitialized_t IsInitialized;
    ProtobufInitializationErrorString_t InitializationErrorString;
    ProtobufMergeFrom_t MergeFrom;
    ProtobufMergePartialFromCodedStream_t MergePartialFromCodedStream;
    ProtobufByteSize_t ByteSize;
    ProtobufSerializeWithCachedSizes_t SerializeWithCachedSizes;
    ProtobufSerializeToArray_t SerializeToArray;
    ProtobufGetCachedSize_t GetCachedSize;
} sc_protobuf_vtable_t;

#endif // STEAMCLIENT_PROTOBUF_VTABLE_H_
