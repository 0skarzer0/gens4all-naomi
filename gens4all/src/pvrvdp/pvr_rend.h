#ifndef PVR_REND_H
#define PVR_REND_H

#include "vdp_data.h"
#include "pvr_cxt.h"


#define PVRVDP_TARGET_LIST PC_PUNCHTHROUGH
//~ #define PVRVDP_TARGET_LIST PC_BLEND_POLY

#define PVR_CODEBOOK_SIZE	(2048)
#define PVRVDP_CRAST_CNT	(2)

typedef struct {
	float left, top, right, bottom;
} PvrUV32;
typedef union {
	struct {
		int topleft, bottomright;
	};
	//~ struct {
		//~ short top, left, bottom, right;
	//~ };
} PvrUV16;
typedef struct {
	char VDPPalette[VDP_PALETTE_CNT][PVR_CODEBOOK_SIZE];
} PVRPaletteCodebook;
typedef struct {
	PVRPaletteCodebook CB[PVRVDP_CRAST_CNT];
	char VRAM[VDP_VRAM_SIZE];
} PVRVRAMData;

//~ extern PVRVRAMData __attribute__ ((aligned(32))) VdpTiles;

extern PvrUV32 __attribute__ ((aligned(32))) TileUV32Table[TILE_DEF_COMBOS*PVRVDP_CRAST_CNT];
extern PvrUV16 __attribute__ ((aligned(32))) TileUV16Table[TILE_DEF_COMBOS*PVRVDP_CRAST_CNT];

extern pvr_context PVRTilesCxtBnk[2][VDP_PALETTE_CNT];
extern pvr_context PVRTilesCxtBnkAlt[2][VDP_PALETTE_CNT];
extern pvr_context PVRTilesSprCxtBnk[2][VDP_PALETTE_CNT];
extern pvr_context PVRTilesSprCxtBnkAlt[2][VDP_PALETTE_CNT];
extern int CurPVRBuff;
extern pvr_context *CurPVRTilesCxt;
extern pvr_context *CurPVRTilesCxtAlt;
extern pvr_context *CurPVRTilesSprCxt;
extern pvr_context *CurPVRTilesSprCxtAlt;
extern pvr_context *CurPVRTilesCxtArr[PVRVDP_CRAST_CNT*2];
extern pvr_ptr_t PVRTilesTexture[2];

extern pvr_ptr_t PVRBGColorTexture[2];
extern pvr_context __attribute__ ((aligned(32))) PVRBgColorCxt[2];

extern void InitTileUVTable(void);
extern void InitPVRTexture(void);
void VRAMToTexture(void *src, void *dst);
extern void DrawTile32PVR_Asm(int tile, void *sq, int color, float topx, float bottomx, float y, float lowz, float hiz, float w, float h);

typedef void (*DrawRow)(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth);


#if 1
	#define DEPTH_LAYER_BG		(1.0f)
	#define DEPTH_LAYER_B_LO	(2.0f)
	#define DEPTH_LAYER_A_LO	(3.0f)
	#define DEPTH_LAYER_S_LO	(4.0f)
	#define DEPTH_LAYER_B_HI	(5.0f)
	#define DEPTH_LAYER_A_HI	(6.0f)
	#define DEPTH_LAYER_MOD_A	(6.5f)
	#define DEPTH_LAYER_MOD_B	(6.75f)
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
#define screen_start_x	(128.0f)
#define SCREEN_START_Y	(128.0f)
#elif 0
#define screen_start_x	(0.0f)
#define SCREEN_START_Y	(0.0f)
#else
extern float screen_start_x;
#define SCREEN_START_Y	(8.0f)
#endif

typedef struct {
	float width, height;
	float scalew, scaleh;
} VDPGeometry;

extern VDPGeometry vdpgeo;

typedef struct {
	int force_cell;
	
} PVRRenderCfg;

#if !VDP_STRETCH_256
	#ifdef VDP_HIRES
	#define TS 16.0f
	#define TSCA 2.0f

	#define TW 16.0f
	#define TH 16.0f
	#define TSW 2.0f
	#define TSH 2.0f
	#else
	#define TS 8.0f
	#define TSCA 1.0f

	#define TW 8.0
	#define TH 8.0
	#define TSW 1.0
	#define TSH 1.0
	#endif
#else
	#define TS 8.0
	#define TSCA 1.0

	#define TW vdpgeo.width
	#define TH 8.0
	#define TSW vdpgeo.scalew
	#define TSH 1.0
#endif

;

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




void PalToCodebook4(short *src, short *dst);
void VRAM8x8ToTexture2(void *src, void *dst);
void VRAM8x16ToTexture(void *src, void *dst);

enum {
	MAP_DRAW_SPLIT2,
	MAP_DRAW_SPLIT2_CELL,
	MAP_DRAW_SPLIT4,
	MAP_DRAW_SPLIT4_CELL,
};
extern int MapADrawFuncId;
extern int MapBDrawFuncId;
extern void NDrawMapA();
extern void NDrawMapB();
extern void NDrawSpritesNR();
extern void NDrawSpritesR();


#endif
