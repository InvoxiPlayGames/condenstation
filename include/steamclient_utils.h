#include "steamclient_protobuf_vtable.h"

void SCUtils_Init();
void *SCUtils_LookupNID(uint32_t nid);
void *SCUtils_LookupSteamAPINID(uint32_t nid);
sc_protobuf_vtable_t *SCUtils_GetProtobufVtable(const char *protobuf_name);
void SCUtils_ReplaceString(const char *source, const char *destination);
uint32_t SCUtils_BaseAddress();
uint32_t SCUtils_BaseAddress_Data();
uint32_t SCUtils_GetFirstBranchTargetAfterInstruction(uint32_t start_address, uint32_t instruction, int scan_length);

// not a steamclient thing but who's counting?
void SCUtils_GetModuleAddresses(const char * module_name, size_t num_segments, uint32_t *seg_addrs, uint32_t *seg_sizes);

uint32_t SCUtils_FindSteamEngine();
uint32_t SCUtils_FindCCMInterfaceConnect();
uint32_t SCUtils_FindCSteamEngineInitCDNCache();
uint32_t SCUtils_FindCUserLogOn();
