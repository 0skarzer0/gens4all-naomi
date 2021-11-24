#include "gens4all.h"
#include "vdp_io.h"
#include "blit.h"

#ifndef U32
	#define U32 unsigned int
#endif

#include "../../../libsh4/sh4.h"
#include "../../../libsh4/sq.h"
#include "../pvrvdp/npvr_vertex.h"
#include "../pvrvdp/pvr_rend.h"

extern unsigned int ScrA_Addr;
extern unsigned int ScrB_Addr;
extern unsigned int Spr_Addr;
extern unsigned char VSRam[256];

extern unsigned int Win_Addr;
extern unsigned int H_Win_Mul;
extern unsigned int Win_X_Pos;
extern unsigned int Win_Y_Pos;

static short *vsram = (short*)&VSRam;


extern unsigned int H_Scroll_Addr;
extern unsigned int H_Scroll_Mask;

extern U32 H_Scroll_CMul;
extern U32 H_Scroll_CMask;
extern U32 V_Scroll_CMask;	//modulo mask of tilemap height in tiles
extern S32 H_Cell;	//screen width in tiles
extern float screen_start_x;

extern float ColorSplitScreen;	//Color split location in PVR screen space

extern int Show_Rasters;

static inline float lerp(float shown, float hidden, float scale)
{
	return shown + scale * (hidden - shown);
}

void DrawTileUnclipped(int tile, void *sq, pvr_context *tex, float topx, float bottomx, float y, float lowz, float hiz, float w, float h) {
	PvrUV16 *uv;
	
	//~ sq = sqCopy32(sq, tex + VDP_GET_PAL(tile));
	//~ sq = sqCopy32(sq, &tex[VDP_GET_PAL(tile)]);
	sq = sqCopy32(sq, &tex[VDP_GET_PAL(tile)]);
	uv = TileUV16Table + tile;
	
	float z = (tile & VDP_PRIO) ? hiz : lowz;
	
	/*
	 0 1
	 3 2 
	*/
	
	pvr_sprite_txr_t *v = sq;
	
	v->flags = PVR_CMD_VERTEX_EOL;
	v->ax = topx;
	v->ay = y;
	v->az = z;
	v->bx = topx+w;
	v->by = y;
	v->bz = z;
	v->cx = bottomx+w;
	sqSubmit64A(v);
	v->cy = y+h;
	v->cz = z;
	v->dx = bottomx;
	v->dy = y+h;
	v->auv = uv->topleft;
	v->buv = sh4Xtrct(uv->bottomright >> 16, uv->topleft << 16);
	v->cuv = uv->bottomright;
	sqSubmit64B(v);
}
static void DrawTileClipped(int tile, void *sq, pvr_context *tex, float topx, float bottomx, float y, float lowz, float hiz, float w, float h, float topsplit, float lowsplit) {
	PvrUV32 *uv = TileUV32Table + tile;
	
	float z = (tile & VDP_PRIO) ? hiz : lowz;
	
	/*
	 1 3
	 0 2
	*/
	
	sq = sqCopy32(sq, &tex[VDP_GET_PAL(tile)]);
	pvr_vertex_t *v = sq;
	
	v->flags = PVR_CMD_VERTEX;
	v->x = bottomx;
	v->y = y+lowsplit*h;
	v->z = z;
	v->u = uv->left;
	v->v = lerp(uv->top, uv->bottom, lowsplit);
	v->argb = 0xffffffff;
	v = sqSubmit32Advance(v);
	
	v->flags = PVR_CMD_VERTEX;
	v->x = topx;
	v->y = y+topsplit*h;
	v->z = z;
	v->u = uv->left;
	v->v = lerp(uv->top, uv->bottom, topsplit);
	v->argb = 0xffffffff;
	v = sqSubmit32Advance(v);
	
	v->x = bottomx+w;
	v->u = uv->right;
	v = sqSubmit32Advance(v);
	
	v->flags = PVR_CMD_VERTEX_EOL;
	v->x = topx+w;
	v->u = uv->right;
	v = sqSubmit32Advance(v);
}

static void DrawTileClippedStretch(int tile, void *sq, pvr_context *tex, float topx, float topy, float bottomx, float bottomy, float lowz, float hiz, float w, float h, float topsplit, float lowsplit) {
	PvrUV32 *uv = TileUV32Table + tile;
	
	float z = (tile & VDP_PRIO) ? hiz : lowz;
	
	/*
	 1 3
	 0 2
	*/
	
	sq = sqCopy32(sq, &tex[VDP_GET_PAL(tile)]);
	pvr_vertex_t *v = sq;
	
	v->flags = PVR_CMD_VERTEX;
	v->x = bottomx;
	v->y = bottomy;
	v->z = z;
	v->u = uv->left;
	v->v = lerp(uv->top, uv->bottom, lowsplit);
	v->argb = 0xffffffff;
	v = sqSubmit32Advance(v);
	
	v->flags = PVR_CMD_VERTEX;
	v->x = topx;
	v->y = topy;
	v->z = z;
	v->u = uv->left;
	v->v = lerp(uv->top, uv->bottom, topsplit);
	v->argb = 0xffffffff;
	v = sqSubmit32Advance(v);
	
	v->x = bottomx+w;
	v->u = uv->right;
	v = sqSubmit32Advance(v);
	
	v->flags = PVR_CMD_VERTEX_EOL;
	v->x = topx+w;
	v->u = uv->right;
	v = sqSubmit32Advance(v);
}

typedef struct {
	pvr_context *tex;
	pvr_context *texspr;
	int tileofs;
	int rowmask;
	float hipriodepth, lowpriodepth;
	float topsplit, lowsplit;
	float tilewidth;
} VDPDrawMode;

static void DrawMapRowUnclipped(VDPDrawMode *mode, short *tmrow, int firstcol, int cols, float scry, float topx, float lowx) {
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int rowmask = mode->rowmask;
	int tileofs = mode->tileofs;
	int tmx = firstcol;
	float lowpriodepth = mode->lowpriodepth;
	float hipriodepth = mode->hipriodepth;
	pvr_context *tex = mode->texspr;
	float topsplit = mode->topsplit, lowsplit = mode->lowsplit;
	
	int x;
	for(x = 0; x < cols; x++) {
		int mapsample = (tmrow[tmx++ & rowmask] & 0xffff) + tileofs;
		DrawTileUnclipped(mapsample, sq, tex, topx, lowx, scry, lowpriodepth, hipriodepth, TW, TH);
		
		topx += TW;
		lowx += TW;
	}
}

static void DrawMapRowUnclipped2(VDPDrawMode *mode, short *tmrow, int firstcol, int cols, float topx, float scry, float lowx, float scry2) {
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int rowmask = mode->rowmask;
	int tileofs = mode->tileofs;
	int tmx = firstcol;
	float lowpriodepth = mode->lowpriodepth;
	float hipriodepth = mode->hipriodepth;
	pvr_context *tex = mode->texspr;
	float topsplit = mode->topsplit, lowsplit = mode->lowsplit;
	
	int x;
	for(x = 0; x < cols; x++) {
		int mapsample = (tmrow[tmx++ & rowmask] & 0xffff) + tileofs;
		DrawTileUnclipped(mapsample, sq, tex, topx, lowx, scry, lowpriodepth, hipriodepth, TS, TS);
		
		topx += TS;
		lowx += TS;
	}
}

static void DrawMapRowClipped2(VDPDrawMode *mode, short *tmrow, int firstcol, int cols, float topx, float topy, float lowx, float lowy) {
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int rowmask = mode->rowmask;
	int tileofs = mode->tileofs;
	int tmx = firstcol;
	float lowpriodepth = mode->lowpriodepth;
	float hipriodepth = mode->hipriodepth;
	pvr_context *tex = mode->tex;
	float topsplit = mode->topsplit, lowsplit = mode->lowsplit;
	
	int x;
	for(x = 0; x < cols; x++) {
		int mapsample = (tmrow[tmx++ & rowmask] & 0xffff) + tileofs;
		DrawTileClippedStretch(mapsample, sq, tex, topx, topy, lowx, lowy, lowpriodepth, hipriodepth, TS, TS, topsplit, lowsplit);
		
		topx += TS;
		lowx += TS;
	}
}

static void DrawMapRowClipped(VDPDrawMode *mode, short *tmrow, int firstcol, int cols, float scry, float topx, float lowx) {
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int rowmask = mode->rowmask;
	int tileofs = mode->tileofs;
	int tmx = firstcol;
	float lowpriodepth = mode->lowpriodepth;
	float hipriodepth = mode->hipriodepth;
	pvr_context *tex = mode->tex;
	float topsplit = mode->topsplit, lowsplit = mode->lowsplit;
	
	int x;
	for(x = 0; x < cols; x++) {
		int mapsample = (tmrow[tmx++ & rowmask] & 0xffff) + tileofs;
		DrawTileClipped(mapsample, sq, tex, topx, lowx, scry, lowpriodepth, hipriodepth, TS, TS, topsplit, lowsplit);
		
		topx += TS;
		lowx += TS;
	}
}

static inline void DrawMapRowClipped3(VDPDrawMode *mode, short *tmrow, int firstcol, int cols, float topx, float topy, float lowx, float lowy) {
	//return;
	extern void DrawRowAsm(VDPDrawMode *mode, short *tmrow, int firstcol, int cols, float topx, float topy, float lowx, float lowy, float topsplit, float lowsplit);
	extern void DrawRowNoClipAsm(VDPDrawMode *mode, short *tmrow, int firstcol, int cols, float topx, float topy, float lowx, float lowy);
	sqSetTargetAddress((void*)PVR_TA_INPUT);
	if (mode->topsplit == 0 && mode->lowsplit == 1) {
		//~ DrawMapRowUnclipped(mode, tmrow, firstcol, cols, topy, topx, lowx);
		DrawRowNoClipAsm(mode, tmrow, firstcol, cols, topx, topy, lowx, lowy);
		//~ DrawRowAsm(mode, tmrow, firstcol, cols, topx, topy, lowx, lowy, 0, 1);
	} else {
		DrawRowAsm(mode, tmrow, firstcol, cols, topx, topy, lowx, lowy, mode->topsplit, mode->lowsplit);
		//~ DrawMapRowClipped2(mode, tmrow, firstcol, cols, topx, topy, lowx, lowy);
		//~ DrawMapRowClipped(mode, tmrow, firstcol, cols, topx, topy, lowx, lowy);
	}
	
}

static int GetHScroll(int line, int map) {
	return (map == 0 ? Raster_Data[line].hscroll_a : Raster_Data[line].hscroll_b)&0xffff;
	
	short *hscrolltable = (short*)H_Scroll_Addr;
	extern int H_Scroll_Mask_Table[];
	int H_Scroll_Mask = H_Scroll_Mask_Table[Raster_Data[line].regs.Set3 & 3];
	return hscrolltable[(line & H_Scroll_Mask) * 2 + map];// & 0x3ff;
}

static int signext(int val, int mask) {
	int signbit = (mask >> 1) ^ mask;
	if (signbit & val)
		return val | ~mask;
	else 
		return val & mask;
}

#define screenheightintiles	(28)
#define screenheightinpixels	(224)

static struct {
	int top, bottom;
	int left, right;
} WindowData;

void UpdateWindow(struct Reg_VDP_Type *regs) {
	int wvp = regs->Win_V_Pos & 0x1f;
	if (regs->Win_V_Pos & 0x80) {
		WindowData.top = wvp*8;
		WindowData.bottom = screenheightintiles*8;
	} else {
		WindowData.top = 0;
		WindowData.bottom = wvp*8;
	}
	
	wvp = regs->Win_H_Pos & 0x1f;
	if (regs->Win_H_Pos & 0x80) {
		WindowData.left = wvp*2*8;
		WindowData.right = H_Cell*8;
	} else {
		WindowData.left = 0;
		WindowData.right = wvp*2*8;
	}
}

//~ #define LINE_BLANKED(line) (!(Raster_Data[(line)].regs.Set2 & 0x40))
#define LINE_BLANKED(line) (Raster_Lines[(line)] & RASTER_LINE_SCREEN_OFF)

#if 0
void NDrawWin() {
	int screenwidthintiles = H_Cell;
	
	int i = 0, j;
	int wintilemapwidthshift = (H_Cell == 32) ? 5 : 6;
	VDPDrawMode drawmodewin = {CurPVRTilesCxt, CurPVRTilesSprCxt, 0, (H_Cell == 32) ? 31 : 63 , DEPTH_LAYER_A_HI+0.1, DEPTH_LAYER_A_LO+0.1, 0, 1, TW};
	
	int last_remaining = 8;
	sqSetTargetAddress((void*)PVR_TA_INPUT);
	
	for(i = 0; i < screenheightinpixels; ) {
		if (LINE_BLANKED(i)) {
			i++;
			continue;
		}
		int right = VDP_STRETCH_256 ? 640 : 320;
		
		
		if (i >= Raster_ColorSplitLine) {
			drawmodewin.tileofs = 0x10000;
			drawmodewin.tex = CurPVRTilesCxtAlt;
			drawmodewin.texspr = CurPVRTilesSprCxtAlt;
		}
		
		UpdateWindow(&Raster_Data[i].regs);
		//TODO move to around here?
		if (i < WindowData.top || i >= WindowData.bottom) {
			
			//~ int tileyofs = (yscroll % 8);
			int map_row = i/8;
			int offset_from_tile_top = (i & 0x7);
			int remaining_tile = 8 - offset_from_tile_top;
			
			float nypos = SCREEN_START_Y + i;
			
			int j, have_raster = 0, first_raster = -1, last_raster = 0;
			int remaining_raster_scan = 0;
			int tile_height = remaining_tile;
			
			drawmodewin.topsplit = offset_from_tile_top / 8.0f;
			
			//TODO raster check should be moved to around above line?
			for(j = 1, remaining_raster_scan = remaining_tile; j < remaining_raster_scan; j++) {
				if (Raster_Lines[i+j] & (RASTER_LINE_WIN_CHANGE|RASTER_LINE_SCREEN_OFF)) {
					remaining_tile = j;
					tile_height = j;
					break;
				}
			}
			if (have_raster) {
				tile_height = remaining_tile = first_raster;
				
			}
			drawmodewin.lowsplit = (offset_from_tile_top + remaining_tile) / 8.0f;
		checkpal:
			//Check for waterline raster
			if (1 && (i < Raster_ColorSplitLine) && ((i + tile_height) > Raster_ColorSplitLine)) {
				//~ float percentage = (float)(Raster_ColorSplitLine - i) / (float)(i + tile_height - Raster_ColorSplitLine);
				float percentage = (float)(Raster_ColorSplitLine - i) / (float)(tile_height );
				//~ printf("L: %i\nCSL %i\nTH: %i\n%f percent\n", i, (int)Raster_ColorSplitLine, tile_height, percentage);
				
				drawmodewin.lowsplit = drawmodewin.topsplit + (drawmodewin.lowsplit - drawmodewin.topsplit) * percentage;
				//~ remaining_tile = Raster_ColorSplitLine - i;
				tile_height = Raster_ColorSplitLine - i;
			}
			
			int tilesdrawn = (WindowData.right - WindowData.left) / 8;
			if (tilesdrawn > 0) {
				float leftx = screen_start_x + WindowData.left;
				float ypos = i+SCREEN_START_Y;
				U32 MapAddr = (((Raster_Data[i].regs.Pat_Win_Adr) & 0x3C) << 10) + (U32)VRam;
				short *tmrow = (short*)MapAddr + ((i/8) << wintilemapwidthshift);

				DrawMapRowClipped3(&drawmodewin, tmrow, WindowData.left/8, tilesdrawn, leftx, ypos, leftx, ypos+tile_height);
			}
			i += tile_height;
		} else {
			if (Show_Rasters)
				draw_rect_wh(right-24, SCREEN_START_Y+i, 8, 1, 15, -1, sqPrepare((void*)PVR_TA_INPUT));
			//This row is a pure window row
			U32 MapAddr = (((Raster_Data[i].regs.Pat_Win_Adr) & 0x3C) << 10) + (U32)VRam;
			short *tmrow = (short*)MapAddr + ((i/8) << wintilemapwidthshift);
			
			//~ DrawMapRowUnclipped(&drawmodewin, tmrow, 0, screenwidthintiles, (i&~0x7)+SCREEN_START_Y, screen_start_x, screen_start_x);
			DrawMapRowClipped3(&drawmodewin, tmrow, 0, screenwidthintiles, screen_start_x, i+SCREEN_START_Y, screen_start_x, i+SCREEN_START_Y+TH);
			i += 8;
			
		}		
	}
}
#else
void NDrawWin() {
	int screenwidthintiles = H_Cell;
	
	int i = 0, j;
	int wintilemapwidthshift = (H_Cell == 32) ? 5 : 6;
	VDPDrawMode drawmodewin = {CurPVRTilesCxt, CurPVRTilesSprCxt, 0, (H_Cell == 32) ? 31 : 63 , DEPTH_LAYER_A_HI+0.1, DEPTH_LAYER_A_LO+0.1, 0, 1, TW};
	
	int last_remaining = 8;
	sqSetTargetAddress((void*)PVR_TA_INPUT);
	
	for(i = 0; i < screenheightinpixels; ) {
		if (LINE_BLANKED(i)) {
			i++;
			continue;
		}
		int right = VDP_STRETCH_256 ? 640 : 320;
		
		
		if (i >= Raster_ColorSplitLine) {
			drawmodewin.tileofs = 0x10000;
			drawmodewin.tex = CurPVRTilesCxtAlt;
			drawmodewin.texspr = CurPVRTilesSprCxtAlt;
		}
		
		UpdateWindow(&Raster_Data[i].regs);
		
		int map_row = i/8;
		int offset_from_tile_top = (i & 0x7);
		int remaining_tile = 8 - offset_from_tile_top;
		
		float nypos = SCREEN_START_Y + i;
		
		int j, have_raster = 0, first_raster = -1, last_raster = 0;
		int remaining_raster_scan = 0;
		int tile_height = remaining_tile;
		
		drawmodewin.topsplit = offset_from_tile_top / 8.0f;
		
		for(j = 1, remaining_raster_scan = remaining_tile; j < remaining_raster_scan; j++) {
			if (Raster_Lines[i+j] & (RASTER_LINE_WIN_CHANGE|RASTER_LINE_SCREEN_OFF)) {
				remaining_tile = j;
				tile_height = j;
				break;
			}
		}
		//~ if (have_raster) {
			//~ tile_height = remaining_tile = first_raster;
			
		//~ }
		drawmodewin.lowsplit = (offset_from_tile_top + remaining_tile) / 8.0f;
		
		//Check for waterline raster
		if ((i < Raster_ColorSplitLine) && ((i + tile_height) > Raster_ColorSplitLine)) {
			float percentage = (float)(Raster_ColorSplitLine - i) / (float)(tile_height );
			
			drawmodewin.lowsplit = drawmodewin.topsplit + (drawmodewin.lowsplit - drawmodewin.topsplit) * percentage;
			tile_height = Raster_ColorSplitLine - i;
		}
		
		if (i < WindowData.top || i >= WindowData.bottom) {
			//Only side window or no window
			int tilesdrawn = (WindowData.right - WindowData.left) / 8;
				
			
			if (tilesdrawn > 0) {
				if (Show_Rasters)
					draw_rect_wh(right-24, SCREEN_START_Y+i, 4, 1, 15, -1, sqPrepare((void*)PVR_TA_INPUT));
				float leftx = screen_start_x + WindowData.left;
				float ypos = i+SCREEN_START_Y;
				U32 MapAddr = (((Raster_Data[i].regs.Pat_Win_Adr) & 0x3C) << 10) + (U32)VRam;
				short *tmrow = (short*)MapAddr + ((i/8) << wintilemapwidthshift);
				
				DrawMapRowClipped3(&drawmodewin, tmrow, WindowData.left/8, tilesdrawn, leftx, ypos, leftx, ypos+tile_height);
			}
			i += tile_height;
		} else {
			if (Show_Rasters)
				draw_rect_wh(right-24, SCREEN_START_Y+i, 8, 1, 15, -1, sqPrepare((void*)PVR_TA_INPUT));
			//This row is a pure window row
			float leftx = screen_start_x;
			float ypos = i+SCREEN_START_Y;
			U32 MapAddr = (((Raster_Data[i].regs.Pat_Win_Adr) & 0x3C) << 10) + (U32)VRam;
			short *tmrow = (short*)MapAddr + ((i/8) << wintilemapwidthshift);
			
			//~ DrawMapRowUnclipped(&drawmodewin, tmrow, 0, screenwidthintiles, (i&~0x7)+SCREEN_START_Y, screen_start_x, screen_start_x);
			//~ DrawMapRowClipped3(&drawmodewin, tmrow, 0, screenwidthintiles, screen_start_x, i+SCREEN_START_Y, screen_start_x, i+SCREEN_START_Y+TH);
			DrawMapRowClipped3(&drawmodewin, tmrow, 0, screenwidthintiles, leftx, ypos, leftx, ypos+tile_height);
			i += tile_height;
			
		}		
	}
}
#endif

//0 = no rasters
//1 = color raster
//2 = scroll raster
//~ #define RASTER_LVL	2
//~ #define CELL_TILT	1
//~ #define AUTO_SPLIT	4

int WindowUsed() {
	int i = VDP_Num_Vis_Lines;
	while(i--) {
		if ((Raster_Data[i].regs.Win_H_Pos | Raster_Data[i].regs.Win_V_Pos) != 0)
			return 1;
	}
	return 0;
}

typedef void (*DrawFunc)();

#define R_MAP_A 0
#define R_MAP_B 1

#define R_NO_WINDOW 0
#define R_ENABLE_WINDOW 1

#define R_NO_TILT 0
#define R_CELL_TILT 1

#define R_NO_RASTER 0
#define R_COLOR_RASTER 1
#define R_FULL_RASTER 2

#define R_NO_SPLIT 0
#define R_SPLIT_2X 4
#define R_SPLIT_4X 2

#define R_LAYER_SHIFT	0
#define R_WINDOW_SHIFT	1
#define R_TILT_SHIFT	2
#define R_SPLITS_SHIFT	3
#define R_RASTER_SHIFT	6
#define R_ENCODE(layer, window, tilt, splits, rast) \
	((layer << R_LAYER_SHIFT) \
	| ((window & (layer==0))<<R_WINDOW_SHIFT) \
	| (tilt<<R_TILT_SHIFT)  \
	| (splits << R_SPLITS_SHIFT) \
	| (rast << R_RASTER_SHIFT))
#define R_DECODE_LAYER(code) (((code) >> R_LAYER_SHIFT) & 1)
#define R_DECODE_WINDOW(code) (((code) >> R_WINDOW_SHIFT) & 1)
#define R_DECODE_TILT(code) (((code) >> R_TILT_SHIFT) & 1)
#define R_DECODE_SPLITS(code) (((code) >> R_SPLITS_SHIFT) & 7)
#define R_DECODE_RASTER(code) (((code) >> R_RASTER_SHIFT) & 3)

#define MY_DRAW_CODE R_ENCODE(R_MAP_A, R_ENABLE_WINDOW, R_CELL_TILT, R_SPLIT_2X, R_FULL_RASTER)
#define DRAW_MAP_FUNC NDrawMapA_WinS2
#include "ntilemap.c"

#define MY_DRAW_CODE R_ENCODE(R_MAP_A, R_NO_WINDOW, R_CELL_TILT, R_SPLIT_2X, R_FULL_RASTER)
#define DRAW_MAP_FUNC NDrawMapA_S2
#include "ntilemap.c"

#define MY_DRAW_CODE R_ENCODE(R_MAP_A, R_ENABLE_WINDOW, R_NO_TILT, R_SPLIT_2X, R_FULL_RASTER)
#define DRAW_MAP_FUNC NDrawMapA_WinCS2
#include "ntilemap.c"

#define MY_DRAW_CODE R_ENCODE(R_MAP_A, R_NO_WINDOW, R_NO_TILT, R_SPLIT_2X, R_FULL_RASTER)
#define DRAW_MAP_FUNC NDrawMapA_CS2
#include "ntilemap.c"

#define MY_DRAW_CODE R_ENCODE(R_MAP_A, R_ENABLE_WINDOW, R_CELL_TILT, R_SPLIT_4X, R_FULL_RASTER)
#define DRAW_MAP_FUNC NDrawMapA_WinS4
#include "ntilemap.c"

#define MY_DRAW_CODE R_ENCODE(R_MAP_A, R_NO_WINDOW, R_CELL_TILT, R_SPLIT_4X, R_FULL_RASTER)
#define DRAW_MAP_FUNC NDrawMapA_S4
#include "ntilemap.c"

#define MY_DRAW_CODE R_ENCODE(R_MAP_A, R_ENABLE_WINDOW, R_NO_TILT, R_SPLIT_4X, R_FULL_RASTER)
#define DRAW_MAP_FUNC NDrawMapA_WinCS4
#include "ntilemap.c"

#define MY_DRAW_CODE R_ENCODE(R_MAP_A, R_NO_WINDOW, R_NO_TILT, R_SPLIT_4X, R_FULL_RASTER)
#define DRAW_MAP_FUNC NDrawMapA_CS4
#include "ntilemap.c"

#define MY_DRAW_CODE R_ENCODE(R_MAP_A, R_NO_WINDOW, R_NO_TILT, R_SPLIT_4X, R_NO_RASTER)
#define DRAW_MAP_FUNC NDrawMapA_CS4NR
#include "ntilemap.c"

int MapADrawFuncId = 0;

static const DrawFunc MapADraw[] = {
	&NDrawMapA_S2,
	&NDrawMapA_CS2,
	&NDrawMapA_S4,
	&NDrawMapA_CS4,
};
static const DrawFunc MapAWinDraw[] = {
	&NDrawMapA_WinS2,
	&NDrawMapA_WinCS2,
	&NDrawMapA_WinS4,
	&NDrawMapA_WinCS4,
};

extern int Sonic_2_Auto_Render;
void NDrawMapA() {
	if (Sonic_2_Auto_Render) {
		//Are we on Sonic 2's special stage?
		if (Ram_68k[0xF600^1] == 0x10) {
			NDrawMapA_CS4NR();
		} else {
			NDrawMapA_CS2();
		}
		return;
	}
	if (MapADrawFuncId < 0)
		return;
	if (MapADrawFuncId >= 4)
		return;
	
	if (WindowUsed()) {
		MapAWinDraw[MapADrawFuncId]();
		extern int Enable_Win;
		if (Enable_Win)
			NDrawWin();
	} else {
		MapADraw[MapADrawFuncId]();
	}
}

#define MY_DRAW_CODE R_ENCODE(R_MAP_B, R_NO_WINDOW, R_CELL_TILT, R_SPLIT_2X, R_FULL_RASTER)
#define DRAW_MAP_FUNC NDrawMapB_S2
#include "ntilemap.c"

#define MY_DRAW_CODE R_ENCODE(R_MAP_B, R_NO_WINDOW, R_NO_TILT, R_SPLIT_2X, R_FULL_RASTER)
#define DRAW_MAP_FUNC NDrawMapB_CS2
#include "ntilemap.c"

#define MY_DRAW_CODE R_ENCODE(R_MAP_B, R_NO_WINDOW, R_CELL_TILT, R_SPLIT_4X, R_FULL_RASTER)
#define DRAW_MAP_FUNC NDrawMapB_S4
#include "ntilemap.c"

#define MY_DRAW_CODE R_ENCODE(R_MAP_B, R_NO_WINDOW, R_NO_TILT, R_SPLIT_4X, R_FULL_RASTER)
#define DRAW_MAP_FUNC NDrawMapB_CS4
#include "ntilemap.c"

#define MY_DRAW_CODE R_ENCODE(R_MAP_B, R_NO_WINDOW, R_NO_TILT, R_SPLIT_2X, R_NO_RASTER)
#define DRAW_MAP_FUNC NDrawMapB_CS2NR
#include "ntilemap.c"

int MapBDrawFuncId = 0;

static const DrawFunc MapBDraw[] = {
	&NDrawMapB_S2,
	&NDrawMapB_CS2,
	&NDrawMapB_S4,
	&NDrawMapB_CS4,
};

void NDrawMapB() {
	if (Sonic_2_Auto_Render) {
		//Are we on Sonic 2's special stage?
		if (Ram_68k[0xF600^1] == 0x10) {
			NDrawMapB_CS2NR();
		} else {
			NDrawMapB_S4();
		}
		return;
	}
	
	if (MapBDrawFuncId < 0)
		return;
	if (MapBDrawFuncId >= 4)
		return;
	MapBDraw[MapBDrawFuncId]();
}

#undef RASTER_LVL
#undef CELL_TILT
#undef AUTO_SPLIT

static void DrawTileCustom(int tile, int ofs, pvr_context *cxts, void *sq, float x, float y, float z, float w, float h) {
	PvrUV16 *uv;
	
	z += (tile < 0) ? DEPTH_LAYER_S_HI : DEPTH_LAYER_S_LO;
	sq = sqCopy32(sq, cxts + (VDP_GET_PAL(tile)));
	uv = TileUV16Table + (tile&0xffff) + ofs;
	
	
	/*
	 0 1
	 3 2 
	*/
	
	pvr_sprite_txr_t *v = sq;
	
	v->flags = PVR_CMD_VERTEX_EOL;
	v->ax = x;
	v->ay = y;
	v->az = z;
	v->bx = x+w;
	v->by = y;
	v->bz = z;
	v->cx = x+w;
	sqSubmit64A(v);
	v->cy = y+h;
	v->cz = z;
	v->dx = x;
	v->dy = y+h;
	v->auv = uv->topleft;
	v->buv = sh4Xtrct(uv->bottomright >> 16, uv->topleft << 16);
	v->cuv = uv->bottomright;
	sqSubmit64B(v);
}

static void DrawTileNoRaster(int tile, void *sq, float x, float y, float z, float w, float h) {
	DrawTileCustom(tile, 0, CurPVRTilesSprCxt, sq, x, y, z, w, h);
}


static inline void DrawTileColorRaster(int tile, void *sq, float x, float y, float z, float w, float h) {
	PvrUV16 *uv;
	//~ tile &= 0xffff;
		//~ DrawTile16PreRaster(tile, sq, color, topx, bottomx, y, lowz, hiz, w, h);
		//~ return;
	if (y+TS < ColorSplitScreen) {
		//Completely over
		DrawTileCustom(tile, 0, CurPVRTilesSprCxt, sq, x, y, z, w, h);
	} else if (y > ColorSplitScreen) {
		//Completely under
		DrawTileCustom(tile, 0x10000, CurPVRTilesSprCxtAlt, sq, x, y, z, w, h);
	} else {
		
		z += (tile < 0) ? DEPTH_LAYER_S_HI : DEPTH_LAYER_S_LO;
		tile &= 0xffff;
		
		PvrUV32 *uv = TileUV32Table + tile;
		sq = sqCopy32(sq, &CurPVRTilesCxt[VDP_GET_PAL(tile)]);
		
		
		/*
		 1 3
		 0 2
		*/
		
		float split = (ColorSplitScreen - y) / TS;
		
		pvr_vertex_t *v = sq;
		
		v->flags = PVR_CMD_VERTEX;
		v->x = x;
		v->y = y+split*h;
		v->z = z;
		v->u = uv->left;
		v->v = lerp(uv->top, uv->bottom, split);
		v->argb = 0xffffffff;
		v = sqSubmit32Advance(v);
		
		v->flags = PVR_CMD_VERTEX;
		v->x = x;
		v->y = y;
		v->z = z;
		v->u = uv->left;
		v->v = uv->top;
		v->argb = 0xffffffff;
		v = sqSubmit32Advance(v);
		
		v->x = x+w;
		v->u = uv->right;
		v = sqSubmit32Advance(v);
		
		v->flags = PVR_CMD_VERTEX_EOL;
		v->x = x+w;
		v->u = uv->right;
		v = sqSubmit32Advance(v);
		
		uv = TileUV32Table + tile + (1<<16);
		v = sqCopy32(v, &CurPVRTilesCxtAlt[VDP_GET_PAL(tile)]);
		
		v->flags = PVR_CMD_VERTEX;
		v->x = x;
		v->y = y+h;
		v->z = z;
		v->u = uv->left;
		v->v = uv->bottom;
		v->argb = 0xffffffff;
		v = sqSubmit32Advance(v);
		
		v->flags = PVR_CMD_VERTEX;
		v->x = x;
		v->y = y+split*h;
		v->z = z;
		v->u = uv->left;
		v->v = lerp(uv->top, uv->bottom, split);
		v->argb = 0xffffffff;
		v = sqSubmit32Advance(v);
		
		v->x = x+w;
		v->u = uv->right;
		v = sqSubmit32Advance(v);
		
		v->flags = PVR_CMD_VERTEX_EOL;
		v->x = x+w;
		v->u = uv->right;
		v = sqSubmit32Advance(v);
	}
}


typedef struct {
	char w, h;
	short ofs;
} SpriteLayout;


#define TILE_DRAW	DrawTileNoRaster
#define DRAW_SPRITE_NAME	DrawSpriteNR
#define DRAW_SPRITES_NAME	NDrawSpritesNR
#include "nsprite.c"
#undef TILE_DRAW
#undef DRAW_SPRITE_NAME
#undef DRAW_SPRITES_NAME

#define TILE_DRAW	DrawTileColorRaster
#define DRAW_SPRITE_NAME	DrawSpriteR
#define DRAW_SPRITES_NAME	NDrawSpritesR
#include "nsprite.c"
#undef TILE_DRAW
#undef DRAW_SPRITE_NAME
#undef DRAW_SPRITES_NAME

