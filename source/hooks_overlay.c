#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <cell/pad.h>

#include "ps3_utilities.h"
#include "ISteamPS3OverlayRender_c.h"

uint8_t whitetexture[4] = {0xff, 0xff, 0xff, 0xff};
ISteamPS3OverlayRenderHost_t *renderHost = NULL;

ISteamPS3OverlayRender_BHostInitialise_t ISteamPS3OverlayRender_BHostInitialise;
bool ISteamPS3OverlayRender_BHostInitialise_Hook(void *thisobj, uint32_t unScreenWidth, uint32_t unScreenHeight, uint32_t unRefreshRate, void *pRenderHost, void *cellFontLib)
{
    _sys_printf("ISteamPS3OverlayRender::BHostInitialise\n");
    _sys_printf("  SteamPS3OverlayRender: %p\n", thisobj);
    _sys_printf("  unScreenWidth: %i\n", unScreenWidth);
    _sys_printf("  unScreenHeight: %i\n", unScreenHeight);
    _sys_printf("  unRefreshRate: %i\n", unRefreshRate);
    _sys_printf("  pRenderHost: %p\n", pRenderHost);
    _sys_printf("  cellFontLib: %p\n", cellFontLib);

    if (renderHost == NULL) {
        renderHost = pRenderHost;

        // empty texture for us to do stuff with
        renderHost->vt->LoadOrUpdateTexture(renderHost, 3001, true, 0, 0, 1, 1, sizeof(whitetexture), whitetexture);
/*
        // example putting qr code into a texture

        QRcode *code = QRcode_encodeString("https://s.team/a/1/youlostthegame", 20, QR_ECLEVEL_Q, QR_MODE_8, 1);
        if (code != NULL) {
            _sys_printf("qrcode rendered %p\n", code);
            _sys_printf("qrcode is %ix%i\n", code->width, code->width);
            _sys_printf("qrcode is %i bytes\n", code->width * code->width);
            _sys_printf("qrcode texture size must be %i\n", (code->width * code->width) * 4);
            int32_t out_tex_size = 0;
            QRcodeToRGBA(code, qrcodetexture, &out_tex_size);
            qrcodewidth = code->width;
            _sys_printf("out_tex_size = %i\n", out_tex_size);
            renderHost->vt->LoadOrUpdateTexture(renderHost, 3002, true, 0, 0, qrcodewidth, qrcodewidth, out_tex_size, qrcodetexture);
            QRcode_free(code);
        }
*/
    }

    return ISteamPS3OverlayRender_BHostInitialise(thisobj, unScreenWidth, unScreenHeight, unRefreshRate, pRenderHost, cellFontLib);
}

ISteamPS3OverlayRender_Render_t ISteamPS3OverlayRender_Render;
void ISteamPS3OverlayRender_Render_Hook(void *thisobj)
{
    ISteamPS3OverlayRender_Render(thisobj);
    //renderHost->vt->DrawTexturedRect(renderHost, 200, 200, 200 + (qrcodewidth * 4), 200 + (qrcodewidth * 4), 0, 0, 1, 1, 3002, 0xFFFFFFFF, 0xFFFFFFFF, k_EOverlayGradientHorizontal);
}

ISteamPS3OverlayRender_BHandleCellPadData_t ISteamPS3OverlayRender_BHandleCellPadData;
bool ISteamPS3OverlayRender_BHandleCellPadData_Hook(void *thisobj, CellPadData *padData)
{
    if ((padData->button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_R3))
        return true;
    return ISteamPS3OverlayRender_BHandleCellPadData(thisobj, padData);
}

void ApplyOverlayHooks(ISteamPS3OverlayRender_t *renderer)
{
    if (renderer != NULL) {
        ISteamPS3OverlayRender_BHostInitialise = renderer->vt->BHostInitialise;
        ISteamPS3OverlayRender_Render = renderer->vt->Render;
        ISteamPS3OverlayRender_BHandleCellPadData = renderer->vt->BHandleCellPadData;
        PS3_Write32(&(renderer->vt->BHostInitialise), (uint32_t)ISteamPS3OverlayRender_BHostInitialise_Hook);
        PS3_Write32(&(renderer->vt->Render), (uint32_t)ISteamPS3OverlayRender_Render_Hook);
        PS3_Write32(&(renderer->vt->BHandleCellPadData), (uint32_t)ISteamPS3OverlayRender_BHandleCellPadData_Hook);
    }
}
