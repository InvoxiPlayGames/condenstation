#ifndef VALVE_PS3_H_
#define VALVE_PS3_H_

#include <stdint.h>
#include <sys/prx.h>

typedef void *(*CreateInterface_t)(const char *name, int *return_code);

// structure passed to the PRX entrypoint when using source addons folder
typedef struct _PS3_LoadAppSystemInterface_Parameters_t {
    // PS3_PrxLoadParametersBase_t
    int size;
    sys_prx_id_t prx_id;
    uint64_t flags;
    uint64_t reserved1[7];
    // PS3_LoadAppSystemInterface_Parameters_t
    CreateInterface_t createInterface;
    uint64_t reserved2[8];
} PS3_LoadAppSystemInterface_Parameters_t;

#endif // VALVE_PS3_H_
