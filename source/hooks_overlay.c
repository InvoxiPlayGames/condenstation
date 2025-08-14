#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <cell/pad.h>

#include "ps3_utilities.h"
#include "ISteamPS3OverlayRender_c.h"
#include "libqrencode_qrencode.h"
#include "tpng.h"
#include "condenstation_logger.h"

#include "generated/condenstation_qr_border.h"

uint8_t qrcodetexture[40000];
uint32_t qrcodewidth = 0;
void QRcodeToRGBA(QRcode *code, uint8_t *out_texture, int32_t *out_texture_size)
{
    *out_texture_size = (code->width * code->width) * 4;
    int texoffset = 0;
    int qroffset = 0;
    for (int x = 0; x < code->width; x++) {
        for (int y = 0; y < code->width; y++) {
            uint8_t qrbyte = code->data[qroffset++];
            if ((qrbyte & 1) == 1) {
                out_texture[texoffset++] = 0x00; //r
                out_texture[texoffset++] = 0x00; //g
                out_texture[texoffset++] = 0x00; //b
                out_texture[texoffset++] = 0xFF; //a
            } else {
                out_texture[texoffset++] = 0xFF; //r
                out_texture[texoffset++] = 0xFF; //g
                out_texture[texoffset++] = 0xFF; //b
                out_texture[texoffset++] = 0xFF; //a
            }
        }
    }
    cdst_log("qroffset = %i\n", qroffset);
    cdst_log("texoffset = %i\n", texoffset);
}

uint8_t whitetexture[4] = {0xff, 0xff, 0xff, 0xff};
ISteamPS3OverlayRenderHost_t *renderHost = NULL;

bool is_displaying_qr = false;

int screenWidth = 1280;
int screenHeight = 720;
int qrDisplaySize = 230;
int qrBorderWidth = 0;
int qrBorderHeight = 0;

ISteamPS3OverlayRender_BHostInitialise_t ISteamPS3OverlayRender_BHostInitialise;
bool ISteamPS3OverlayRender_BHostInitialise_Hook(void *thisobj, uint32_t unScreenWidth, uint32_t unScreenHeight, uint32_t unRefreshRate, void *pRenderHost, void *cellFontLib)
{
    cdst_log("ISteamPS3OverlayRender::BHostInitialise\n");
    cdst_log("  SteamPS3OverlayRender: %p\n", thisobj);
    cdst_log("  unScreenWidth: %i\n", unScreenWidth);
    cdst_log("  unScreenHeight: %i\n", unScreenHeight);
    cdst_log("  unRefreshRate: %i\n", unRefreshRate);
    cdst_log("  pRenderHost: %p\n", pRenderHost);
    cdst_log("  cellFontLib: %p\n", cellFontLib);

    screenWidth = unScreenWidth;
    screenHeight = unScreenHeight;

    if (renderHost == NULL) {
        renderHost = pRenderHost;
    }

    return ISteamPS3OverlayRender_BHostInitialise(thisobj, unScreenWidth, unScreenHeight, unRefreshRate, pRenderHost, cellFontLib);
}

ISteamPS3OverlayRender_Render_t ISteamPS3OverlayRender_Render;
void ISteamPS3OverlayRender_Render_Hook(void *thisobj)
{
    ISteamPS3OverlayRender_Render(thisobj);
    if (is_displaying_qr) {
        int borderPosX = (screenWidth / 2) - (qrBorderWidth / 2);
        int borderPosY = (screenHeight / 2) - (qrBorderHeight / 2);
        renderHost->vt->DrawTexturedRect(renderHost, borderPosX, borderPosY, borderPosX + qrBorderWidth, borderPosY + qrBorderHeight, 0, 0, 1, 1, 3001, 0xFFFFFFFF, 0xFFFFFFFF, k_EOverlayGradientHorizontal);
        int qrPosX = (screenWidth / 2) - (qrDisplaySize / 2);
        int qrPosY = (screenHeight / 2) - (qrDisplaySize / 2);
        renderHost->vt->DrawTexturedRect(renderHost, qrPosX, qrPosY, qrPosX + qrDisplaySize, qrPosY + qrDisplaySize, 0, 0, 1, 1, 3002, 0xFFFFFFFF, 0xFFFFFFFF, k_EOverlayGradientHorizontal);
    }
}

void QRoverlay_stop_displaying_qr();
ISteamPS3OverlayRender_BHandleCellPadData_t ISteamPS3OverlayRender_BHandleCellPadData;
bool ISteamPS3OverlayRender_BHandleCellPadData_Hook(void *thisobj, CellPadData *padData)
{
    if (is_displaying_qr) {
        if ((padData->button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CIRCLE)) {
            QRoverlay_stop_displaying_qr();
        }
        return true;
    }
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

void cancel_qr_auth_session();

// QRoverlay_start_displaying_qr("https://s.team/a/1/youlostthegame")
void QRoverlay_start_displaying_qr(const char *url)
{
    // render out the qr code
    QRcode *code = QRcode_encodeString(url, 0, QR_ECLEVEL_Q, QR_MODE_8, 1);
    if (code != NULL) {
        cdst_log("qrcode rendered %p\n", code);
        cdst_log("qrcode is %ix%i\n", code->width, code->width);
        cdst_log("qrcode is %i bytes\n", code->width * code->width);
        cdst_log("qrcode texture size must be %i\n", (code->width * code->width) * 4);
        int32_t out_tex_size = 0;
        QRcodeToRGBA(code, qrcodetexture, &out_tex_size);
        qrcodewidth = code->width;
        cdst_log("out_tex_size = %i\n", out_tex_size);
        renderHost->vt->LoadOrUpdateTexture(renderHost, 3002, true, 0, 0, qrcodewidth, qrcodewidth, out_tex_size, qrcodetexture);
        QRcode_free(code);
    } else {
        cdst_log("failed to render qr code");
        return;
    }
    // render out the border
    uint8_t *png_data = tpng_get_rgba(condenstation_qr_border_png, condenstation_qr_border_png_len, &qrBorderWidth, &qrBorderHeight);
    if (png_data != NULL) {
        cdst_log("loaded RBGA data of %ix%i\n", qrBorderWidth, qrBorderHeight);
        renderHost->vt->LoadOrUpdateTexture(renderHost, 3001, true, 0, 0, qrBorderWidth, qrBorderHeight, qrBorderWidth * qrBorderHeight * 4, png_data);
        free(png_data);
    } else {
        cdst_log("failed to render png data");
        return;
    }
    // set the value saying we're displaying a qr
    is_displaying_qr = true;
}

void QRoverlay_stop_displaying_qr()
{
    is_displaying_qr = false;
    renderHost->vt->DeleteTexture(renderHost, 3001); // border
    renderHost->vt->DeleteTexture(renderHost, 3002); // qr code itself
    cancel_qr_auth_session();
}
