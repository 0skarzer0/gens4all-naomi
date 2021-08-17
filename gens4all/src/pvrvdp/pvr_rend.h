#ifndef PVR_REND_H
#define PVR_REND_H

#include "vdp_data.h"
#include "pvr_cxt.h"

#define PVRVDP_DBL_BUFF

#define PVRVDP_TARGET_LIST PC_PUNCHTHROUGH
//~ #define PVRVDP_TARGET_LIST PC_BLEND_POLY

typedef struct {
	float left, top, right, bottom;
} PvrUV32;
typedef struct {
	int topleft, bottomright;
} PvrUV16;
typedef struct {
	char PaletteCodebook[2048*VDP_PALETTE_CNT];
	char VRAM[VDP_VRAM_SIZE];
} PVRVRAMData;

extern PVRVRAMData __attribute__ ((aligned(32))) VdpTiles;

extern PvrUV32 __attribute__ ((aligned(32))) TileUV32Table[TILE_DEF_COMBOS];
extern PvrUV16 __attribute__ ((aligned(32))) TileUV16Table[TILE_DEF_COMBOS];
#ifdef PVRVDP_DBL_BUFF
extern pvr_context PVRTilesCxtBnk[2][VDP_PALETTE_CNT];
extern pvr_context PVRTilesSprCxtBnk[2][VDP_PALETTE_CNT];
extern int CurPVRBuff;
extern pvr_context *CurPVRTilesCxt;
extern pvr_context *CurPVRTilesSprCxt;
extern pvr_ptr_t PVRTilesTexture[2];
#else
extern pvr_ptr_t PVRTilesTexture;
extern pvr_context PVRTilesCxt[VDP_PALETTE_CNT];
extern pvr_context PVRTilesSprCxt[VDP_PALETTE_CNT];
#endif

extern void InitTileUVTable(void);
extern void InitPVRTexture(void);
void VRAMToTexture(void *src, void *dst);
extern void DrawTile32PVR_Asm(int tile, void *sq, int color, float topx, float bottomx, float y, float lowz, float hiz, float w, float h);



#define TS 8.0f
#if 1
	#define DEPTH_LAYER_BG		(1.0f)
	#define DEPTH_LAYER_B_LO	(2.0f)
	#define DEPTH_LAYER_A_LO	(3.0f)
	#define DEPTH_LAYER_S_LO	(4.0f)
	#define DEPTH_LAYER_B_HI	(5.0f)
	#define DEPTH_LAYER_A_HI	(6.0f)
	#define DEPTH_LAYER_S_HI	(7.0f)
#else
	#define DEPTH_LAYER_B_LO	(0.7f)
	#define DEPTH_LAYER_A_LO	(0.6f)
	#define DEPTH_LAYER_S_LO	(0.5f)
	#define DEPTH_LAYER_B_HI	(0.4f)
	#define DEPTH_LAYER_A_HI	(0.3f)
	#define DEPTH_LAYER_S_HI	(0.2f)
#endif

#if 0
#define SCREEN_START_X	(128.0f)
#define SCREEN_START_Y	(128.0f)
#else
#define SCREEN_START_X	(0.0f)
#define SCREEN_START_Y	(0.0f)
#endif

void * draw_rect(float x0, float y0, float x1, float y1, float z, unsigned int argb, void *dst);
void DrawVRAMPVRManual(float scrx, float scry, float depth);
void DrawMapsPVR();

void DrawMapWindow(float lowpriodepth, float hipriodepth);
void DrawMapPVR(int layer, float lowpriodepth, float hipriodepth);
void DrawLayerCellPVR();
void DrawSprites();
void * draw_rect_wh(float x, float y, float w, float h, float z, unsigned int argb, void *dst);


void PalToCodebook(short *src, short *dst);
void purge_cache();

#endif
