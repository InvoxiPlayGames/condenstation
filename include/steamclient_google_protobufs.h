#include "std_string.h"

typedef void (*WireFormatLite_WriteString_t)(int protoId, std_basic_string *string, void *codedOutputStream);
typedef void (*WireFormatLite_WriteBool_t)(int protoId, bool boolean, void *codedOutputStream);
typedef void (*WireFormatLite_WriteBytes_t)(int protoId, std_basic_string *bytes, void *codedOutputStream);
typedef void (*WireFormatLite_WriteUInt32_t)(int protoId, unsigned int uint_32, void *codedOutputStream);
typedef void (*WireFormatLite_WriteInt32_t)(int protoId, int int_32, void *codedOutputStream);
typedef void (*WireFormatLite_WriteFixed32_t)(int protoId, unsigned int uint_32, void *codedOutputStream);
typedef void (*WireFormatLite_WriteFixed64_t)(int protoId, unsigned long long uint_64, void *codedOutputStream);

typedef int (*CodedOutputStream_VarintSize32Fallback_t)(int varInt);
