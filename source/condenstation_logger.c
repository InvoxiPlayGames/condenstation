#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <cell/rtc.h>
#include <cell/cell_fs.h>

bool log_to_file = false;
bool log_to_debugger = true;

#define LOG_FILE_NAME "/dev_hdd0/tmp/condenstation.log"

int _sys_printf(const char *fmt, ...);
int _sys_vprintf(const char *fmt, va_list args);

FILE *file_log = NULL;

int cdst_log(const char *fmt, ...)
{
    va_list args;
    if (log_to_debugger) {
        va_start(args, fmt);
        _sys_vprintf(fmt, args);
        va_end(args);
    }
    if (log_to_file && file_log != NULL) {
        va_start(args, fmt);
        vfprintf(file_log, fmt, args);
        fflush(file_log);
        va_end(args);
    }
}

void condenstation_log_init() {
    CellFsStat st;
    log_to_file = cellFsStat(LOG_FILE_NAME, &st) == CELL_FS_SUCCEEDED;

    if (log_to_file) {
        file_log = fopen(LOG_FILE_NAME, "a");
    }

    cdst_log("---\ncondenstation beta 2 active\n---\n");
}
