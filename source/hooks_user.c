#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <cell/pad.h>

#include "ps3_utilities.h"
#include "ISteamUser_c.h"

ISteamUser_t *steamUser = NULL;

extern int _sys_printf(char *fmt, ...);

void ApplyUserHooks(ISteamUser_t *su)
{
    if (su != NULL) {
        steamUser = su;
    }
}
