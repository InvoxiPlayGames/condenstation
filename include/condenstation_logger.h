#include <stdbool.h>

extern bool log_to_file;
extern bool log_to_debugger;

int cdst_log(const char *fmt, ...);
void condenstation_log_init();
