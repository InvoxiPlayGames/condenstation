#ifndef CELLHTTPHELPER_H_
#define CELLHTTPHELPER_H_

#include <stdbool.h>
#include <cell/http.h>

CellHttpClientId current_cellHttp_client;
bool is_cellHttp_init;

int condenstation_init_cellHttp();

#endif // CELLHTTPHELPER_H_
