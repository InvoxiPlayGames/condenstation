#include "steamclient_protobuf_vtable.h"

void SCUtils_Init();
void *SCUtils_LookupNID(uint32_t nid);
sc_protobuf_vtable_t *SCUtils_GetProtobufVtable(const char *protobuf_name);
uint32_t SCUtils_BaseAddress();
uint32_t SCUtils_GetFirstBranchTargetAfterInstruction(uint32_t start_address, uint32_t instruction, int scan_length);
