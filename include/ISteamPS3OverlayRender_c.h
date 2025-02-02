#ifndef ISTEAMPS3OVERLAYRENDER_C_H_
#define ISTEAMPS3OVERLAYRENDER_C_H_
#include <stdint.h>

typedef enum _EOverlayGradientDirection_t
{
	k_EOverlayGradientHorizontal = 1,
	k_EOverlayGradientVertical = 2,
	k_EOverlayGradientNone = 3,
} EOverlayGradientDirection_t;

// Helpers for fetching individual color components from ARGB packed DWORD colors Steam PS3 overlay renderer uses.
#define STEAM_COLOR_RED( color ) \
	(int)(((color)>>16)&0xff)

#define STEAM_COLOR_GREEN( color ) \
	(int)(((color)>>8)&0xff)

#define STEAM_COLOR_BLUE( color ) \
	(int)((color)&0xff)

#define STEAM_COLOR_ALPHA( color ) \
	(int)(((color)>>24)&0xff)

typedef bool (*ISteamPS3OverlayRender_BHostInitialise_t)(void *thisobj, uint32_t unScreenWidth, uint32_t unScreenHeight, uint32_t unRefreshRate, void *pRenderHost, void *cellFontLib);
typedef void (*ISteamPS3OverlayRender_Render_t)(void *thisobj);
typedef bool (*ISteamPS3OverlayRender_BHandleCellPadData_t)(void *thisobj, void *padData);
typedef bool (*ISteamPS3OverlayRender_BResetInputState_t)(void *thisobj);

typedef struct _ISteamPS3OverlayRender_vt {
    ISteamPS3OverlayRender_BHostInitialise_t BHostInitialise;
    ISteamPS3OverlayRender_Render_t Render;
    ISteamPS3OverlayRender_BHandleCellPadData_t BHandleCellPadData;
    ISteamPS3OverlayRender_BResetInputState_t BResetInputState;
} ISteamPS3OverlayRender_vt;

typedef struct _ISteamPS3OverlayRender_t {
    ISteamPS3OverlayRender_vt *vt;
} ISteamPS3OverlayRender_t;

typedef void (*ISteamPS3OverlayRenderHost_DrawTexturedRect_t)(void *thisobj, int x0, int y0, int x1, int y1, float u0, float v0, float u1, float v1, int32_t iTextureID, uint32_t colorStart, uint32_t colorEnd, EOverlayGradientDirection_t eDirection);
typedef void (*ISteamPS3OverlayRenderHost_LoadOrUpdateTexture_t)(void *thisobj, int32_t iTextureID, bool bIsFullTexture, int x0, int y0, uint32_t uWidth, uint32_t uHeight, int32_t iBytes, char *pData);
typedef void (*ISteamPS3OverlayRenderHost_DeleteTexture_t)(void *thisobj, int32_t iTextureID);
typedef void (*ISteamPS3OverlayRenderHost_DeleteAllTextures_t)(void *thisobj);

typedef struct _ISteamPS3OverlayRenderHost_vt {
    ISteamPS3OverlayRenderHost_DrawTexturedRect_t DrawTexturedRect;
    ISteamPS3OverlayRenderHost_LoadOrUpdateTexture_t LoadOrUpdateTexture;
    ISteamPS3OverlayRenderHost_DeleteTexture_t DeleteTexture;
    ISteamPS3OverlayRenderHost_DeleteAllTextures_t DeleteAllTextures;
} ISteamPS3OverlayRenderHost_vt;

typedef struct _ISteamPS3OverlayRenderHost_t {
    ISteamPS3OverlayRenderHost_vt *vt;
} ISteamPS3OverlayRenderHost_t;

#endif // ISTEAMPS3OVERLAYRENDER_C_H_
