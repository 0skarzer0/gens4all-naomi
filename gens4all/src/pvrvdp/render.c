#include "gens4all.h"
#include "blit.h"

#ifndef U32
	#define U32 unsigned int
#endif

#include "../../../libsh4/sh4.h"
#include "../../../libsh4/sq.h"
//#include "../pvrvdp/pvr_cxt.h"
#include "../pvrvdp/npvr_vertex.h"
#include "../pvrvdp/pvr_rend.h"

//~ #define DrawTile32PVR(a...) DrawTile16PreRaster(a)
#define DrawTile32PVR(a...) DrawTile16PVR_C(a)
//~ #define DrawTile32PVR(a...) DrawTile32PVR_Asm(a)

float screen_start_x = 0;

float ColorSplitScreen;	//Color split location in PVR screen space

void DrawTile32PVR_C(int tile, void *sq, int color, float topx, float bottomx, float y, float lowz, float hiz, float w, float h) {
	PvrUV32 *uv = TileUV32Table + (tile & 0xffff);
	
	sq = sqCopy32(sq, &CurPVRTilesCxt[VDP_GET_PAL(tile)]);
	
	float z = (tile < 0) ? hiz : lowz;
	tile &= 0xffff;
	
	/*
	 1 3
	 0 2
	*/
	
	pvr_vertex_t *v = sq;
	
	v->flags = PVR_CMD_VERTEX;
	v->x = bottomx;
	v->y = y+h;
	v->z = z;
	v->u = uv->left;
	v->v = uv->bottom;
	v->argb = color;
	v = sqSubmit32Advance(v);
	
	v->flags = PVR_CMD_VERTEX;
	v->x = topx;
	v->y = y;
	v->z = z;
	v->u = uv->left;
	v->v = uv->top;
	v->argb = color;
	v = sqSubmit32Advance(v);
	
	v->x = bottomx+w;
	v->u = uv->right;
	v = sqSubmit32Advance(v);
	
	v->flags = PVR_CMD_VERTEX_EOL;
	v->x = topx+w;
	v->u = uv->right;
	v = sqSubmit32Advance(v);
}

void DrawTile16PreRaster(int tile, void *sq, int color, float topx, float bottomx, float y, float lowz, float hiz, float w, float h) {
	PvrUV16 *uv;
	sq = sqCopy32(sq, &CurPVRTilesSprCxt[VDP_GET_PAL(tile)]);
	uv = TileUV16Table + (tile & 0xffff);
	
	float z = (tile < 0) ? hiz : lowz;
	
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

void DrawTile16PostRaster(int tile, void *sq, int color, float topx, float bottomx, float y, float lowz, float hiz, float w, float h) {
	PvrUV16 *uv;
	
	sq = sqCopy32(sq, &CurPVRTilesSprCxtAlt[VDP_GET_PAL(tile)]);
	uv = TileUV16Table + (tile & 0xffff) + (1<<16);
	
	float z = (tile < 0) ? hiz : lowz;
	
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

void DrawTile16PVR_C(int tile, void *sq, int color, float topx, float bottomx, float y, float lowz, float hiz, float w, float h) {
	PvrUV16 *uv;
	
	//~ sq = sqCopy32(sq, &CurPVRTilesSprCxt[VDP_GET_PAL(tile) + (topx>Raster_ColorSplitLine?4:0)]);
	if (y>ColorSplitScreen) {
		sq = sqCopy32(sq, &CurPVRTilesSprCxtAlt[VDP_GET_PAL(tile)]);
		uv = TileUV16Table + (tile & 0xffff) + (1<<16);
	} else {
		sq = sqCopy32(sq, &CurPVRTilesSprCxt[VDP_GET_PAL(tile)]);
		uv = TileUV16Table + (tile & 0xffff);
	}
	//~ if (topx > Raster_ColorSplitLine) {
		//~ printf("Topx %f\n", topx); fflush(stdout);
	//~ }
	
	float z = (tile < 0) ? hiz : lowz;
	
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

static inline float lerp(float shown, float hidden, float scale)
{
	return shown + scale * (hidden - shown);
}

void DrawTileSplitTop(int tile, void *sq, pvr_context *tex, int color, float topx, float bottomx, float y, float lowz, float hiz, float w, float h, float split) {
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
	v->y = y+split*h;
	v->z = z;
	v->u = uv->left;
	v->v = lerp(uv->top, uv->bottom, split);
	v->argb = color;
	v = sqSubmit32Advance(v);
	
	v->flags = PVR_CMD_VERTEX;
	v->x = topx;
	v->y = y;
	v->z = z;
	v->u = uv->left;
	v->v = uv->top;
	v->argb = color;
	v = sqSubmit32Advance(v);
	
	v->x = bottomx+w;
	v->u = uv->right;
	v = sqSubmit32Advance(v);
	
	v->flags = PVR_CMD_VERTEX_EOL;
	v->x = topx+w;
	v->u = uv->right;
	v = sqSubmit32Advance(v);
}

void DrawTileSplitBottom(int tile, void *sq, pvr_context *tex, int color, float topx, float bottomx, float y, float lowz, float hiz, float w, float h, float split) {
	PvrUV32 *uv = TileUV32Table + tile;
	
	float z = (tile & VDP_PRIO) ? hiz : lowz;
	
	/*
	 1 3
	 0 2
	*/
	
	sq = sqCopy32(sq, &tex[VDP_GET_PAL(tile)]);
	//~ sq = sqCopy32(sq, tex);
	pvr_vertex_t *v = sq;
	
	v->flags = PVR_CMD_VERTEX;
	v->x = bottomx;
	v->y = y+h;
	v->z = z;
	v->u = uv->left;
	v->v = uv->bottom;
	v->argb = color;
	v = sqSubmit32Advance(v);
	
	v->flags = PVR_CMD_VERTEX;
	v->x = topx;
	v->y = y+split*h;
	v->z = z;
	v->u = uv->left;
	v->v = lerp(uv->top, uv->bottom, split);
	v->argb = color;
	v = sqSubmit32Advance(v);
	
	v->x = bottomx+w;
	v->u = uv->right;
	v = sqSubmit32Advance(v);
	
	v->flags = PVR_CMD_VERTEX_EOL;
	v->x = topx+w;
	v->u = uv->right;
	v = sqSubmit32Advance(v);
}

void DrawTileSplitMid(int tile, void *sq, pvr_context *tex, int color, float topx, float bottomx, float y, float lowz, float hiz, float w, float h, float topsplit, float lowsplit) {
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
	v->argb = color;
	v = sqSubmit32Advance(v);
	
	v->flags = PVR_CMD_VERTEX;
	v->x = topx;
	v->y = y+topsplit*h;
	v->z = z;
	v->u = uv->left;
	v->v = lerp(uv->top, uv->bottom, topsplit);
	v->argb = color;
	v = sqSubmit32Advance(v);
	
	v->x = bottomx+w;
	v->u = uv->right;
	v = sqSubmit32Advance(v);
	
	v->flags = PVR_CMD_VERTEX_EOL;
	v->x = topx+w;
	v->u = uv->right;
	v = sqSubmit32Advance(v);
}



void DrawTile16Split(int tile, void *sq, int color, float topx, float bottomx, float y, float lowz, float hiz, float w, float h) {
	PvrUV16 *uv;
	
		DrawTile16PreRaster(tile, sq, color, topx, bottomx, y, lowz, hiz, w, h);
		return;
	if (y+TS < ColorSplitScreen) {
		//Completely over
		DrawTile16PreRaster(tile, sq, color, topx, bottomx, y, lowz, hiz, w, h);
	} else if (y > ColorSplitScreen) {
		//Completely under
		DrawTile16PostRaster(tile, sq, color, topx, bottomx, y, lowz, hiz, w, h);
	} else {
		PvrUV32 *uv = TileUV32Table + (tile & 0xffff);
		
		sq = sqCopy32(sq, &CurPVRTilesCxt[VDP_GET_PAL(tile)]);
		
		float z = (tile < 0) ? hiz : lowz;
		tile &= 0xffff;
		
		/*
		 1 3
		 0 2
		*/
		
		float split = (ColorSplitScreen - y) / TS;
		
		pvr_vertex_t *v = sq;
		
		v->flags = PVR_CMD_VERTEX;
		v->x = bottomx;
		v->y = y+split*h;
		v->z = z;
		v->u = uv->left;
		v->v = lerp(uv->top, uv->bottom, split);
		v->argb = color;
		v = sqSubmit32Advance(v);
		
		v->flags = PVR_CMD_VERTEX;
		v->x = topx;
		v->y = y;
		v->z = z;
		v->u = uv->left;
		v->v = uv->top;
		v->argb = color;
		v = sqSubmit32Advance(v);
		
		v->x = bottomx+w;
		v->u = uv->right;
		v = sqSubmit32Advance(v);
		
		v->flags = PVR_CMD_VERTEX_EOL;
		v->x = topx+w;
		v->u = uv->right;
		v = sqSubmit32Advance(v);
		
		uv = TileUV32Table + (tile & 0xffff) + (1<<16);
		sq = sqCopy32(sq, &CurPVRTilesCxtAlt[VDP_GET_PAL(tile)]);
		
		v->flags = PVR_CMD_VERTEX;
		v->x = bottomx;
		v->y = y+h;
		v->z = z;
		v->u = uv->left;
		v->v = uv->bottom;
		v->argb = color;
		v = sqSubmit32Advance(v);
		
		v->flags = PVR_CMD_VERTEX;
		v->x = topx;
		v->y = y+split*h;
		v->z = z;
		v->u = uv->left;
		v->v = lerp(uv->top, uv->bottom, split);
		v->argb = color;
		v = sqSubmit32Advance(v);
		
		v->x = bottomx+w;
		v->u = uv->right;
		v = sqSubmit32Advance(v);
		
		v->flags = PVR_CMD_VERTEX_EOL;
		v->x = topx+w;
		v->u = uv->right;
		v = sqSubmit32Advance(v);
	}
}

void * draw_rect(float x0, float y0, float x1, float y1, float z, unsigned int argb, void *dst) {
	pvr_context spr;
	pc_set_default_sprite(&spr);
	pc_set_sprite_color_packed(&spr, argb);
	pc_set_list(&spr, PVRVDP_TARGET_LIST);
	pc_set_modified(&spr, 1);
	pc_set_modifier_full(&spr, 0);
	dst = sqCopy32(dst, &spr);
	
	pvr_vertex64s *v = dst;
	pv_set_cmd_vertex_eos(v);
	pv_set_submit_rect_ccw(v, x0, y0, x1, y1, z);
	return sqSubmit64BAdvance(v);
}
short VdpColorToARGB4444(short in) {
	int red = in & 0xE;
	int green = in & 0xE0;
	int blue = (in & 0xE00);
	return 0xf000 | (red << 8) | green | (blue >> 8);
}
void PalToCodebook(short *src, short *dst) {
	short colors[VDP_PALETTE_SIZE];
	
	//Convert pal to ARGB4444
	int i;
	for(i = 0; i < VDP_PALETTE_SIZE; i++) {
		colors[i] = VdpColorToARGB4444(src[i]);
	}
	
	colors[0] &= ~(0xf<<12);	//set color 0 to zero alpha

	i = 256;
	while(i--) {
		if (1) {
			dst[i*4 + 0] = colors[i >> 4];
			dst[i*4 + 1] = colors[i >> 4];
			dst[i*4 + 2] = colors[i & 0xf];
			dst[i*4 + 3] = colors[i & 0xf];
		} else {
			dst[i*4 + 0] = colors[i & 0xf];
			dst[i*4 + 1] = colors[i & 0xf];
			dst[i*4 + 2] = colors[i >> 4];
			dst[i*4 + 3] = colors[i >> 4];
		}
	}
}
void PalToCodebook2(short *src, int *dst) {
	int colors[VDP_PALETTE_SIZE];
	
	//Convert pal to ARGB4444
	int i;
	for(i = 0; i < VDP_PALETTE_SIZE; i++) {
		int t = VdpColorToARGB4444(src[i]) & 0xffff;
		colors[i] = t | (t<<16);
	}
	
	colors[0] &= 0x0fff0fff;	//set color 0 to zero alpha

	i = 256;
	int sq = 4;
	dst += 512;
	while(i--) {
		*--dst = colors[i >> 4];
		*--dst = colors[i & 0xf];
		
		if (--sq) {
			sqSubmit32(dst);
			sq = 4;
		}
	}
}

void PalToCodebook3(short *src, int *dst) {
	int colors[VDP_PALETTE_SIZE];
	
	//Convert pal to ARGB4444
	int i;
	for(i = 0; i < VDP_PALETTE_SIZE; i++) {
		int t = VdpColorToARGB4444(src[i]) & 0xffff;
		colors[i] = t | (t<<16);
	}
	
	colors[0] &= 0x0fff0fff;	//set color 0 to zero alpha

	i = 255;
	//~ int sq = 4;
	dst += 511;
	do {
		sh4CachelineAllocate(dst, colors[i >> 4]);
		*--dst = colors[i & 0xf];
		i--;
		*--dst = colors[i >> 4];
		*--dst = colors[i & 0xf];
		i--;
		*--dst = colors[i >> 4];
		*--dst = colors[i & 0xf];
		i--;
		*--dst = colors[i >> 4];
		*--dst = colors[i & 0xf];
		i--;
		
		//~ if (--sq) {
			sqSubmit32(dst--);
			//~ sq = 4;
		//~ }
	} while(i > 0);
}

int testcolor = 0;
void DrawVRAMPVRManual(float scrx, float scry, float depth) {
	int y, x;
	float yf = scry, xf = scrx;
	void *pc = sqPrepare((void*)PVR_TA_INPUT);
	for(y = 0; y < 32; y++) {
		xf = scrx;
		for(x = 0; x < 32; x++) {
			//DrawTile32PVR_C((y*64+x%64) | (testcolor<<(VDP_PALSHIFT-2)), pc, -1, xf, xf, yf, 0.5, 0.5, 8, 8);
			//~ DrawTile32PVR(y*64+x, pc, -1, xf, xf, yf, depth, depth, 8, 8);
			DrawTile16PreRaster(y*64+x, pc, -1, xf, xf, yf, depth, depth, TS, TS);
			xf += 8.0f;
		}
		yf += 8.0f;
	}
}
void purge_cache() {
	//static char __attribute__ ((aligned (32))) flusharea[16*1024];
	static char __attribute__ ((aligned (32))) flusharea[16*1024];
	int i;
	
	for(i = 0; i < 16*1024; i += 32) {
		sh4CachelineAllocate(flusharea + i, 0);
		sh4Invalidate(flusharea + i);
	}
}

void purge_cache_oci() {
	//static char __attribute__ ((aligned (32))) flusharea[16*1024];
	static char __attribute__ ((aligned (32))) flusharea_low[8*1024];
	char * flushareaa = OCI_BANK0(flusharea_low);
	char * flushareab = OCI_BANK1(flusharea_low);
	int i = 8*1024/32;
	
	do {
		sh4CachelineAllocate(flushareaa, 0);
		sh4Invalidate(flushareaa);
		flushareaa += 32;
		sh4CachelineAllocate(flushareab, 0);
		sh4Invalidate(flushareab);
		flushareab += 32;
	} while(--i);
}

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
static int GetHScroll(int line, int map) {
	short *hscrolltable = (short*)H_Scroll_Addr;
	return hscrolltable[(line & H_Scroll_Mask) * 2 + map];// & 0x3ff;
	//~ short *hscrolltable = (short*)H_Scroll_Addr;
	//~ extern int H_Scroll_Mask_Table[];
	//~ int H_Scroll_Mask = H_Scroll_Mask_Table[Raster_Data[line].regs.Set3 & 3];
	//~ return hscrolltable[(line & H_Scroll_Mask) * 2 + map];// & 0x3ff;
}

static int signext(int val, int mask) {
	int signbit = (mask >> 1) ^ mask;
	if (signbit & val)
		return val | ~mask;
	else 
		return val & mask;
}

extern U32 H_Scroll_CMul;
extern U32 H_Scroll_CMask;
extern U32 V_Scroll_CMask;	//modulo mask of tilemap height in tiles
extern S32 H_Cell;	//screen width in tiles
	
void DrawMapRowPVR(short *tmrow, int layer, int line, float scry, float lowpriodepth, float hipriodepth) {
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int tilemapwidthmask = H_Scroll_CMask;
	int tilemapwidthshift = H_Scroll_CMul;
	int mapsample;
	
	int screenwidthintiles = H_Cell+1;
	int l = -GetHScroll(line < 0 ? 0 : line, layer);
	int llow = -GetHScroll(((line+7) > 223) ? 223 : line+7, layer);
	
	int tmx, cols;
	int slope = signext(l - llow, tilemapwidthmask);
	float leftxmap, leftxmaplow;
	if (slope < 0) {
		//top line is to the left of bottom line
		tmx = l / 8;
		cols = screenwidthintiles - slope/8;
		leftxmap = screen_start_x - (l % 8);
		leftxmaplow = leftxmap + slope;
	} else {
		tmx = llow / 8;
		cols = screenwidthintiles + slope/8;
		leftxmaplow = screen_start_x - (llow % 8);
		leftxmap = leftxmaplow - slope;
	}
	
	tmx--;
	leftxmap *= TSCA;
	leftxmaplow *= TSCA;
	leftxmap -= TS;
	leftxmaplow -= TS;
	
	int x;
	if (scry < ColorSplitScreen) {
		for(x = 0; x <= cols; x++) {
			mapsample = tmrow[tmx++ & tilemapwidthmask];
			//~ DrawTile16Split(mapsample, sq, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS);
			//~ DrawTile32PVR(mapsample, sq, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS);
			DrawTile16PreRaster(mapsample, sq, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS);
			
			leftxmap += TS;
			leftxmaplow += TS;
		}
	} else {
		for(x = 0; x <= cols; x++) {
			mapsample = tmrow[tmx++ & tilemapwidthmask];
			//~ DrawTile16Split(mapsample, sq, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS);
			//~ DrawTile32PVR(mapsample, sq, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS);
			DrawTile16PostRaster(mapsample, sq, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS);
			
			leftxmap += TS;
			leftxmaplow += TS;
		}
	}
}

void DrawMapRowPVRCell(short *tmrow, int layer, int line, float scry, float lowpriodepth, float hipriodepth) {
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int tilemapwidthmask = H_Scroll_CMask;
	int tilemapwidthshift = H_Scroll_CMul;
	int mapsample;
	
	int screenwidthintiles = H_Cell+1;
	int l = -GetHScroll(line < 0 ? 0 : line, layer);
	int llow = l;
	
	int tmx, cols;
	int slope = signext(l - llow, tilemapwidthmask);
	float leftxmap, leftxmaplow;
	
	tmx = llow / 8;
	cols = screenwidthintiles + slope/8;
	leftxmaplow = screen_start_x - (llow % 8);
	leftxmap = leftxmaplow - slope;
	
	tmx--;
	leftxmap *= TSCA;
	leftxmaplow *= TSCA;
	leftxmap -= TS;
	leftxmaplow -= TS;
	
	int x;
	if (scry < ColorSplitScreen) {
		for(x = 0; x <= cols; x++) {
			mapsample = tmrow[tmx++ & tilemapwidthmask];
			//~ DrawTile16Split(mapsample, sq, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS);
			//~ DrawTile32PVR(mapsample, sq, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS);
			DrawTile16PreRaster(mapsample, sq, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS);
			
			leftxmap += TS;
			leftxmaplow += TS;
		}
	} else {
		for(x = 0; x <= cols; x++) {
			mapsample = tmrow[tmx++ & tilemapwidthmask];
			//~ DrawTile16Split(mapsample, sq, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS);
			//~ DrawTile32PVR(mapsample, sq, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS);
			DrawTile16PostRaster(mapsample, sq, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS);
			
			leftxmap += TS;
			leftxmaplow += TS;
		}
	}
}
	
void DrawMapRowColorSplit(short *tmrow, int layer, int line, float scry, float lowpriodepth, float hipriodepth) {
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int tilemapwidthmask = H_Scroll_CMask;
	int tilemapwidthshift = H_Scroll_CMul;
	int mapsample;
	
	int screenwidthintiles = H_Cell+1;
	int l = -GetHScroll(line < 0 ? 0 : line, layer);
	int llow = -GetHScroll(((line+7) > 223) ? 223 : line+7, layer);
	
	int tmx, cols;
	int slope = signext(l - llow, tilemapwidthmask);
	float leftxmap, leftxmaplow;
	if (slope < 0) {
		//top line is to the left of bottom line
		tmx = l / 8;
		cols = screenwidthintiles - slope/8;
		leftxmap = screen_start_x - (l % 8);
		leftxmaplow = leftxmap + slope;
	} else {
		tmx = llow / 8;
		cols = screenwidthintiles + slope/8;
		leftxmaplow = screen_start_x - (llow % 8);
		leftxmap = leftxmaplow - slope;
	}
	
	tmx--;
	leftxmap *= TSCA;
	leftxmaplow *= TSCA;
	leftxmap -= TS;
	leftxmaplow -= TS;
	
	int x;
	for(x = 0; x <= cols; x++) {
		mapsample = tmrow[tmx++ & tilemapwidthmask];
		DrawTile16Split(mapsample, sq, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS);
		//~ DrawTile32PVR(mapsample, sq, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS);
		
		leftxmap += TS;
		leftxmaplow += TS;
	}
}

void DrawMapRowSplitTop(short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth, float split) {
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int tilemapwidthmask = H_Scroll_CMask;
	int tilemapwidthshift = H_Scroll_CMul;
	int mapsample;
	
	int screenwidthintiles = H_Cell+1;
	int toplineofs = line;
	int lowlineofs = line + 8 * split - 2;	//Why does this need to be -2 and not -1?
	if (toplineofs < 0)
		toplineofs = 0;
	if (lowlineofs < toplineofs)
		lowlineofs = toplineofs;
	if (lowlineofs > 223)
		lowlineofs = 223;
	
	int l = -GetHScroll(toplineofs, layer);
	int llow = -GetHScroll(lowlineofs, layer);
	
	int tmx, cols;
	int slope = signext(l - llow, tilemapwidthmask);
	float leftxmap, leftxmaplow;
	if (slope < 0) {
		//top line is to the left of bottom line
		tmx = l / 8;
		cols = screenwidthintiles - slope/8;
		leftxmap = screen_start_x - (l % 8);
		leftxmaplow = leftxmap + slope;
	} else {
		tmx = llow / 8;
		cols = screenwidthintiles + slope/8;
		leftxmaplow = screen_start_x - (llow % 8);
		leftxmap = leftxmaplow - slope;
	}
	
	tmx--;
	leftxmap *= TSCA;
	leftxmaplow *= TSCA;
	leftxmap -= TS;
	leftxmaplow -= TS;
	
	int x;
	for(x = 0; x <= cols; x++) {
		mapsample = (tmrow[tmx++ & tilemapwidthmask] & 0xffff) + tileofs;
		DrawTileSplitTop(mapsample, sq, CurPVRTilesCxt, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS, split);
		
		leftxmap += TS;
		leftxmaplow += TS;
	}
}

void DrawMapRowSplitBottom(short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth, float split) {
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int tilemapwidthmask = H_Scroll_CMask;
	int tilemapwidthshift = H_Scroll_CMul;
	int mapsample;
	
	int screenwidthintiles = H_Cell+1;
	int toplineofs = line + 8 * split - 1;
	int lowlineofs = line + 7;
	if (toplineofs < 0)
		toplineofs = 0;
	if (toplineofs > 223)
		toplineofs = 223;
	if (lowlineofs > 223)
		lowlineofs = 223;
	
	int l = -GetHScroll(toplineofs, layer);
	int llow = -GetHScroll(lowlineofs, layer);
	
	int tmx, cols;
	int slope = signext(l - llow, tilemapwidthmask);
	float leftxmap, leftxmaplow;
	if (slope < 0) {
		//top line is to the left of bottom line
		tmx = l / 8;
		cols = screenwidthintiles - slope/8;
		leftxmap = screen_start_x - (l % 8);
		leftxmaplow = leftxmap + slope;
	} else {
		tmx = llow / 8;
		cols = screenwidthintiles + slope/8;
		leftxmaplow = screen_start_x - (llow % 8);
		leftxmap = leftxmaplow - slope;
	}
	
	tmx--;
	leftxmap *= TSCA;
	leftxmaplow *= TSCA;
	leftxmap -= TS;
	leftxmaplow -= TS;
	
	int x;
	for(x = 0; x <= cols; x++) {
		mapsample = (tmrow[tmx++ & tilemapwidthmask] & 0xffff) + tileofs;
		DrawTileSplitBottom(mapsample, sq, CurPVRTilesCxtAlt, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS, split);
		
		leftxmap += TS;
		leftxmaplow += TS;
	}
}


void DrawMapRowSplitMid(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth, float topsplit, float lowsplit) {
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int tilemapwidthmask = H_Scroll_CMask;
	int tilemapwidthshift = H_Scroll_CMul;
	int mapsample;
	
	int screenwidthintiles = H_Cell+1;
	int toplineofs = line + 8 * topsplit;
	int lowlineofs = line + 8 * lowsplit-1;
	if (toplineofs < 0)
		toplineofs = 0;
	if (toplineofs > 223)
		toplineofs = 223;
	if (lowlineofs > 223)
		lowlineofs = 223;
	
	int l = -GetHScroll(toplineofs, layer);
	int llow = -GetHScroll(lowlineofs, layer);
	
	int tmx, cols;
	int slope = signext(l - llow, tilemapwidthmask);
	float leftxmap, leftxmaplow;
	if (slope < 0) {
		//top line is to the left of bottom line
		tmx = l / 8;
		cols = screenwidthintiles - slope/8;
		leftxmap = screen_start_x - (l % 8);
		leftxmaplow = leftxmap + slope;
	} else {
		tmx = llow / 8;
		cols = screenwidthintiles + slope/8;
		leftxmaplow = screen_start_x - (llow % 8);
		leftxmap = leftxmaplow - slope;
	}
	
	tmx--;
	leftxmap *= TSCA;
	leftxmaplow *= TSCA;
	leftxmap -= TS;
	leftxmaplow -= TS;
	
	int x;
	for(x = 0; x <= cols; x++) {
		mapsample = (tmrow[tmx++ & tilemapwidthmask] & 0xffff) + tileofs;
		DrawTileSplitMid(mapsample, sq, tex, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS, topsplit, lowsplit);
		
		leftxmap += TS;
		leftxmaplow += TS;
	}
}

void DrawMapRowSplitMidCell(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth, float topsplit, float lowsplit) {
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int tilemapwidthmask = H_Scroll_CMask;
	int tilemapwidthshift = H_Scroll_CMul;
	int mapsample;
	
	int screenwidthintiles = H_Cell+1;
	int toplineofs = line + 8 * topsplit;
	int lowlineofs = line + 8 * lowsplit-1;
	if (toplineofs < 0)
		toplineofs = 0;
	if (toplineofs > 223)
		toplineofs = 223;
	if (lowlineofs > 223)
		lowlineofs = 223;
	
	int l = -GetHScroll(toplineofs, layer);
	
	int tmx, cols;
	//top line is to the left of bottom line
	tmx = l / 8;
	cols = screenwidthintiles;
	float leftxmap = screen_start_x - (l % 8);
	
	tmx--;
	leftxmap *= TSCA;
	leftxmap -= TS;
	
	int x;
	for(x = 0; x <= cols; x++) {
		mapsample = (tmrow[tmx++ & tilemapwidthmask] & 0xffff) + tileofs;
		DrawTileSplitMid(mapsample, sq, tex, -1, leftxmap, leftxmap, scry, lowpriodepth, hipriodepth, TS, TS, topsplit, lowsplit);
		
		leftxmap += TS;
	}
}

void DrawMapRow_NoLineSroll(short *tmrow, int layer, int line, float scry, float lowpriodepth, float hipriodepth) {
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int tilemapwidthmask = H_Scroll_CMask;
	int tilemapwidthshift = H_Scroll_CMul;
	int mapsample;
	
	int screenwidthintiles = H_Cell;
	int l = -(GetHScroll(line, layer));
	int llow = -(GetHScroll(line+7, layer));
	int tmx = l / 8;
	float leftxmap = -(l % 8);
	float leftxmaplow = leftxmap;
	
	int x;
	for(x = 0; x <= (screenwidthintiles+0); x++) {
		mapsample = tmrow[tmx++ & tilemapwidthmask];
		DrawTile32PVR(mapsample, sq, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS);
		
		leftxmap += TS;
		leftxmaplow += TS;
	}
}


void DrawRowFastCell(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth) {
	DrawMapRowPVRCell(tmrow, layer, line, scry, lowpriodepth, hipriodepth);
}

void DrawRowFast(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth) {
	DrawMapRowPVR(tmrow, layer, line, scry, lowpriodepth, hipriodepth);
}
void DrawRowHalfTile(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth) {
	//~ DrawMapRowPVR(tmrow, layer, topline, yf, lowpriodepth, hipriodepth);
	DrawMapRowSplitMid(tex, tmrow, layer, line, 0, scry, lowpriodepth, hipriodepth, 0, .5);
	DrawMapRowSplitMid(tex, tmrow, layer, line, 0, scry, lowpriodepth, hipriodepth, .5, 1);
}

void DrawRowQuarterTile(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth) {
	//~ DrawMapRowPVR(tmrow, layer, line, scry, lowpriodepth, hipriodepth);
	DrawMapRowSplitMid(tex, tmrow, layer, line, 0, scry, lowpriodepth, hipriodepth, 0.00, 0.25);
	DrawMapRowSplitMid(tex, tmrow, layer, line, 0, scry, lowpriodepth, hipriodepth, 0.25, 0.50);
	DrawMapRowSplitMid(tex, tmrow, layer, line, 0, scry, lowpriodepth, hipriodepth, 0.50, 0.75);
	DrawMapRowSplitMid(tex, tmrow, layer, line, 0, scry, lowpriodepth, hipriodepth, 0.75, 1.00);
}

void DrawMapRowAutoSplit1(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth) {
	int i, splits = 0;
	int s = GetHScroll(line, layer);
	//~ int dir = 0;
	int firstsplit = 0;
	
	for(i = 1; i < 8; i++) {
		int s2 = GetHScroll(line+i, layer);
		if (s != s2) {
			splits++;
			s = s2;
			if (firstsplit == 0) {
				firstsplit = i;
			}
		}
	}
	if (splits == 0) {
		DrawMapRowPVR(tmrow, layer, line, scry, lowpriodepth, hipriodepth);
	} else if (splits == 1) {
		float splitpoint = firstsplit / 8.0f;
		DrawMapRowSplitMid(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, 0, splitpoint);
		DrawMapRowSplitMid(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, splitpoint, 1);
	} else {
		DrawMapRowPVR(tmrow, layer, line, scry, lowpriodepth, hipriodepth);
	}
}

void DrawMapRowAutoSplit2(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth) {
	int i, splits = 0;
	int s = GetHScroll(line, layer);
	//~ int dir = 0;
	int firstsplit = 0;
	
	for(i = 1; i < 8; i++) {
		int s2 = GetHScroll(line+i, layer);
		if (s != s2) {
			splits++;
			s = s2;
			if (firstsplit == 0) {
				firstsplit = i;
			}
		}
	}
	if (splits == 0) {
		DrawMapRowPVR(tmrow, layer, line, scry, lowpriodepth, hipriodepth);
	} else if (splits == 1) {
		float splitpoint = firstsplit / 8.0f;
		DrawMapRowSplitMid(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, 0, splitpoint);
		DrawMapRowSplitMid(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, splitpoint, 1);
	} else {
		DrawMapRowSplitMid(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, 0.00, 0.50);
		DrawMapRowSplitMid(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, 0.50, 1.00);
	}
}

void DrawMapRowAutoSplit1Cell(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth) {
	int i, splits = 0;
	int s = GetHScroll(line, layer);
	//~ int dir = 0;
	int firstsplit = 0;
	
	for(i = 1; i < 8; i++) {
		int s2 = GetHScroll(line+i, layer);
		if (s != s2) {
			splits++;
			s = s2;
			if (firstsplit == 0) {
				firstsplit = i;
			}
		}
	}
	if (splits == 0) {
		DrawMapRowPVRCell(tmrow, layer, line, scry, lowpriodepth, hipriodepth);
	} else if (splits == 1) {
		float splitpoint = firstsplit / 8.0f;
		DrawMapRowSplitMidCell(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, 0, splitpoint);
		DrawMapRowSplitMidCell(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, splitpoint, 1);
	} else {
		DrawMapRowPVRCell(tmrow, layer, line, scry, lowpriodepth, hipriodepth);
	}
}

void DrawMapRowAutoSplit2Cell(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth) {
	int i, splits = 0;
	int s = GetHScroll(line, layer);
	//~ int dir = 0;
	int firstsplit = 0;
	
	for(i = 1; i < 8; i++) {
		int s2 = GetHScroll(line+i, layer);
		if (s != s2) {
			splits++;
			s = s2;
			if (firstsplit == 0) {
				firstsplit = i;
			}
		}
	}
	if (splits == 0) {
		DrawMapRowPVRCell(tmrow, layer, line, scry, lowpriodepth, hipriodepth);
	} else if (splits == 1) {
		float splitpoint = firstsplit / 8.0f;
		DrawMapRowSplitMidCell(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, 0, splitpoint);
		DrawMapRowSplitMidCell(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, splitpoint, 1);
	} else {
		DrawMapRowSplitMidCell(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, 0.00, 0.50);
		DrawMapRowSplitMidCell(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, 0.50, 1.00);
	}
}

void DrawMapRowAutoSplit4Cell(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth) {
	int i, splits = 0;
	int s = GetHScroll(line, layer);
	//~ int dir = 0;
	int firstsplit = 0;
	
	for(i = 1; i < 8; i++) {
		int s2 = GetHScroll(line+i, layer);
		if (s != s2) {
			splits++;
			s = s2;
			if (firstsplit == 0) {
				firstsplit = i;
			}
		}
	}
	if (splits == 0) {
		DrawMapRowPVRCell(tmrow, layer, line, scry, lowpriodepth, hipriodepth);
	} else if (splits == 1) {
		float splitpoint = firstsplit / 8.0f;
		DrawMapRowSplitMidCell(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, 0, splitpoint);
		DrawMapRowSplitMidCell(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, splitpoint, 1);
	} else {
		DrawMapRowSplitMidCell(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, 0.00, 0.25);
		DrawMapRowSplitMidCell(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, 0.25, 0.50);
		DrawMapRowSplitMidCell(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, 0.50, 0.75);
		DrawMapRowSplitMidCell(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, 0.75, 1.00);
	}
}

void DrawMapRowAutoSplit4(pvr_context *tex, short *tmrow, int layer, int line, int tileofs, float scry, float lowpriodepth, float hipriodepth) {
	int i, splits = 0;
	int s = GetHScroll(line, layer);
	//~ int dir = 0;
	int firstsplit = 0;
	
	for(i = 1; i < 8; i++) {
		int s2 = GetHScroll(line+i, layer);
		if (s != s2) {
			splits++;
			s = s2;
			if (firstsplit == 0) {
				firstsplit = i;
			}
		}
	}
	if (splits == 0) {
		DrawMapRowPVR(tmrow, layer, line, scry, lowpriodepth, hipriodepth);
	} else if (splits == 1) {
		float splitpoint = firstsplit / 8.0f;
		DrawMapRowSplitMid(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, 0, splitpoint);
		DrawMapRowSplitMid(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, splitpoint, 1);
	} else {
		DrawMapRowSplitMid(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, 0.00, 0.25);
		DrawMapRowSplitMid(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, 0.25, 0.50);
		DrawMapRowSplitMid(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, 0.50, 0.75);
		DrawMapRowSplitMid(tex, tmrow, layer, line, tileofs, scry, lowpriodepth, hipriodepth, 0.75, 1.00);
	}
}


DrawRow DrawRowB = DrawMapRowAutoSplit2;
DrawRow DrawRowA = DrawMapRowAutoSplit2;

void DrawMapPVR(int layer, float lowpriodepth, float hipriodepth) {
	ColorSplitScreen = Raster_ColorSplitLine + SCREEN_START_Y;
	int x, y;
	float yf = SCREEN_START_Y, xf;
	
	int tilemapwidthshift = H_Scroll_CMul;
	//~ int tilemapwidthshift = 6;
	int tilemapheightmask = V_Scroll_CMask; //tilemapheight-1;
	
	//The S3 file select screen uses 128x32 maps, but a window for the bg
	if (VDP_Reg.Win_V_Pos == 0x80 && layer == 0) {
		return;
	}
	
	int screenwidthintiles = H_Cell;
	int screenheightintiles = 28;
	
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int mapsample;
	
	short *mapaddr = layer ? (short*)ScrB_Addr : (short*)ScrA_Addr;
	int yscroll = vsram[layer] & 0x3ff;
	//yf = -(yscroll & 0x7);
	yf = SCREEN_START_Y - (yscroll % 8);
	yf *= TSCA;
	int yscrollofs = - (yscroll % 8);
	
	for(y = 0; y <= screenheightintiles; y++) {
		xf = screen_start_x;
		short *tmrow = mapaddr + (((yscroll/8 + y) & tilemapheightmask) << tilemapwidthshift);
		int topline = yscrollofs+y*8;
		
		if ((topline+8) < Raster_ColorSplitLine) {
			if (layer == 0)
				DrawRowA(CurPVRTilesCxt, tmrow, layer, topline, 0, yf, lowpriodepth, hipriodepth);
			else
				DrawRowB(CurPVRTilesCxt, tmrow, layer, topline, 0, yf, lowpriodepth, hipriodepth);
		} else if (topline > Raster_ColorSplitLine) {
			if (layer == 0)
				DrawRowA(CurPVRTilesCxtAlt, tmrow, layer, topline, 0x10000, yf, lowpriodepth, hipriodepth);
			else
				DrawRowB(CurPVRTilesCxtAlt, tmrow, layer, topline, 0x10000, yf, lowpriodepth, hipriodepth);
		} else {
			float split = (ColorSplitScreen - yf) / TS;
			DrawMapRowSplitTop(tmrow, layer, topline, 0, yf, lowpriodepth, hipriodepth, split);
			DrawMapRowSplitBottom(tmrow, layer, topline, 0x10000, yf, lowpriodepth, hipriodepth, split);
		}
		//~ DrawMapRow_NoLineSroll(tmrow, layer, y*8, yf, lowpriodepth, hipriodepth);
		yf += TS;
	}
}

void DrawMapPVRFine(int layer, float lowpriodepth, float hipriodepth) {
	int x, y;
	float yf = SCREEN_START_Y, xf;
	
	int tilemapwidthshift = H_Scroll_CMul;
	int tilemapheightmask = V_Scroll_CMask; //tilemapheight-1;
	
	int screenwidthintiles = H_Cell;
	int screenheightintiles = 28;
	
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int mapsample;
	
	short *mapaddr = layer ? (short*)ScrB_Addr : (short*)ScrA_Addr;
	int yscroll = vsram[layer];
	//yf = -(yscroll & 0x7);
	yf = SCREEN_START_Y - (yscroll % 8);
	yf *= TSCA;
	int yscrollofs = - (yscroll % 8);
	
	for(y = 0; y <= screenheightintiles; y++) {
		xf = screen_start_x;
		short *tmrow = mapaddr + (((yscroll/8 + y) & tilemapheightmask) << tilemapwidthshift);
		int topline = yscrollofs+y*8;
		
		if ((topline+8) < Raster_ColorSplitLine) {
			//~ DrawMapRowPVR(tmrow, layer, topline, yf, lowpriodepth, hipriodepth);
			
			//~ DrawMapRowSplitMid(tmrow, layer, topline, 0, yf, lowpriodepth, hipriodepth, 0, 3.0f/8);
			//~ DrawMapRowSplitMid(tmrow, layer, topline, 0, yf, lowpriodepth, hipriodepth, 3.0f/8, 5.0f/8);
			//~ DrawMapRowSplitMid(tmrow, layer, topline, 0, yf, lowpriodepth, hipriodepth, 5.0f/8, 8.0f/8);
			
			DrawMapRowSplitMid(CurPVRTilesCxt, tmrow, layer, topline, 0, yf, lowpriodepth, hipriodepth, 0, .25);
			DrawMapRowSplitMid(CurPVRTilesCxt, tmrow, layer, topline, 0, yf, lowpriodepth, hipriodepth, .25, .5);
			DrawMapRowSplitMid(CurPVRTilesCxt, tmrow, layer, topline, 0, yf, lowpriodepth, hipriodepth, .5, .75);
			DrawMapRowSplitMid(CurPVRTilesCxt, tmrow, layer, topline, 0, yf, lowpriodepth, hipriodepth, .75, 1);
			
			//~ DrawMapRowSplitTop(tmrow, layer, topline, 0, yf, lowpriodepth, hipriodepth, 0.5);
			//~ DrawMapRowSplitBottom(tmrow, layer, topline, 0, yf, lowpriodepth, hipriodepth, 0.5);
		} else if (topline > Raster_ColorSplitLine) {
			DrawMapRowPVR(tmrow, layer, topline, yf, lowpriodepth, hipriodepth);
			//~ DrawMapRowSplitMid(CurPVRTilesCxtAlt, tmrow, layer, topline, 0x10000, yf, lowpriodepth, hipriodepth, 0, .5);
			//~ DrawMapRowSplitMid(CurPVRTilesCxtAlt, tmrow, layer, topline, 0x10000, yf, lowpriodepth, hipriodepth, .5, 1);
		} else {
			//~ DrawMapRowColorSplit(tmrow, layer, topline, yf, lowpriodepth, hipriodepth);
			float split = (Raster_ColorSplitLine - yf) / TS;
			DrawMapRowSplitTop(tmrow, layer, topline, 0, yf, lowpriodepth, hipriodepth, split);
			DrawMapRowSplitBottom(tmrow, layer, topline, 0x10000, yf, lowpriodepth, hipriodepth, split);
		}
		//~ DrawMapRow_NoLineSroll(tmrow, layer, y*8, yf, lowpriodepth, hipriodepth);
		yf += TS;
	}
}

void DrawMapPVR2(int layer, float lowpriodepth, float hipriodepth) {
	int x, y;
	float yf = SCREEN_START_Y, xf;
	
//	int tilemapwidth = 64;
//	int tilemapheight = 32;
	//tilemapwidth = VDP_Reg.
	
	extern U32 H_Scroll_CMul;
	extern U32 H_Scroll_CMask;
	extern U32 V_Scroll_CMask;	//modulo mask of tilemap height in tiles
	extern S32 H_Cell;	//screen width in tiles
	
	int tilemapwidthmask = H_Scroll_CMask; //tilemapwidth-1;
	int tilemapwidthshift = H_Scroll_CMul;
	int tilemapheightmask = V_Scroll_CMask; //tilemapheight-1;
	
	
	int screenwidthintiles = H_Cell;
	int screenheightintiles = 28;
	
	
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int mapsample;
	
	short *mapaddr = layer ? (short*)ScrB_Addr : (short*)ScrA_Addr;
	int yscroll = vsram[layer];
	//yf = -(yscroll & 0x7);
	yf = SCREEN_START_Y - (yscroll % 8);
	
	for(y = 0; y <= screenheightintiles; y++) {
		xf = screen_start_x;
		//int l = -(GetHScroll(y * 8, layer)) - 8;
		int l = -(GetHScroll(y * 8, layer));
		//float leftxmap = -(l & 0x7);
		float leftxmap = -(l % 8);
		//short *tmrow = mapaddr + ((yscroll/8 + y) & tilemapheightmask) * tilemapwidth;
		short *tmrow = mapaddr + (((yscroll/8 + y) & tilemapheightmask) << tilemapwidthshift);
		int tmx = l / 8;
		
		float leftxmaplow = leftxmap;
		//int llow = -(GetHScroll(y * 8 + 7, layer));
		//leftxmaplow = leftxmap + l - llow;

		for(x = 0; x <= (screenwidthintiles+0); x++) {
			mapsample = tmrow[tmx++ & tilemapwidthmask];
			DrawTile32PVR(mapsample, sq, -1, leftxmap+xf, leftxmaplow+xf, yf, lowpriodepth, hipriodepth, TS, TS);
			
			xf += TS;
		}
		yf += TS;
	}
}
extern U32 H_Scroll_CMul;
extern U32 H_Scroll_CMask;
extern U32 V_Scroll_CMask;	//modulo mask of tilemap height in tiles
extern S32 H_Cell;	//screen width in tiles

void DrawMapWindow(float lowpriodepth, float hipriodepth) {
	int x, y;
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int win_enable;
	short *winaddr = (short*)Win_Addr;
	
	int screenwidthintiles = H_Cell;
	int screenheightintiles = 28;
	
	int tilemapwidthmask = (H_Cell == 32) ? 31 : 63;
	int tilemapwidthshift = (H_Cell == 32) ? 5 : 6;
	int tilemapheightmask = 31;
	
	int wintop = 0, winbottom = screenheightintiles;
	int winleft = 0, winright = screenwidthintiles;
	int wvp = VDP_Reg.Win_V_Pos & 0x1f;
	int wdown = VDP_Reg.Win_V_Pos >> 7;
	
	//~ if (!wvp)
		//~ return;
	
	int yscroll = 0;//vsram[layer];
	float yf = SCREEN_START_Y;// - (yscroll % 8);
	if (wdown) {
		wintop = wvp;
		winbottom = screenheightintiles;
	} else {
		wintop = 0;
		winbottom = wvp;
	}
	yf += wintop*TS;
//	printf("(%i, %i) - (%i, %i)\n", winleft, wintop, winright, winbottom);
	for(y = wintop; y < winbottom; y++) {
		float xf = screen_start_x;
		short *tmrow = winaddr + (y << tilemapwidthshift);
		float leftxmap = 0;
		float leftxmaplow = leftxmap;
		for(x = winleft; x < winright; x++) {
			//int mapsample = tmrow[x & tilemapwidthmask];
			int mapsample = winaddr[(y << tilemapwidthshift) + x];
			DrawTile32PVR(mapsample, sq, -1, leftxmap+xf, leftxmaplow+xf, yf, lowpriodepth, hipriodepth, TS, TS);
			
			xf += TS;
		}
		yf += TS;
	}
#if 0
	for(y = wintop; y < winbottom; y++) {
		yf = y * TS;
		float xf = screen_start_x;
		
		int l = -(GetHScroll(y * 8, 0));
		float leftxmap = -(l % 8);
		float leftxmaplow = leftxmap;
		short *tmrow = winaddr + (((yscroll/8 + y) & tilemapheightmask) << tilemapwidthshift);
		int tmx = 0;
		
		for(x = winleft; x < winright; x++) {
			int mapsample = tmrow[tmx++ & tilemapwidthmask];
			DrawTile32PVR(mapsample, sq, -1, leftxmap+xf, leftxmaplow+xf, yf, lowpriodepth, hipriodepth, TS, TS);
			
			xf += TS;
		}
		yf += TS;
	}
#endif
	
}

void DrawMapPVRWindow(int layer, float lowpriodepth, float hipriodepth) {
	int x, y;
	float yf = SCREEN_START_Y, xf;
	
//	int tilemapwidth = 64;
//	int tilemapheight = 32;
	//tilemapwidth = VDP_Reg.
	
	int tilemapwidthmask = H_Scroll_CMask; //tilemapwidth-1;
	int tilemapwidthshift = H_Scroll_CMul;
	int tilemapheightmask = V_Scroll_CMask; //tilemapheight-1;
	
	
	int screenwidthintiles = H_Cell;
	int screenheightintiles = 28;
	
	
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int mapsample;
	
	short *mapaddr = layer ? (short*)ScrB_Addr : (short*)ScrA_Addr;
	short *winaddr = (short*)Win_Addr;
	int yscroll = vsram[layer];
	//yf = -(yscroll & 0x7);
	yf = SCREEN_START_Y - (yscroll % 8);
	
	for(y = 0; y <= screenheightintiles; y++) {
		xf = screen_start_x;
		//int l = -(GetHScroll(y * 8, layer)) - 8;
		int l = -(GetHScroll(y * 8, layer));
		//float leftxmap = -(l & 0x7);
		float leftxmap = -(l % 8);
		//short *tmrow = mapaddr + ((yscroll/8 + y) & tilemapheightmask) * tilemapwidth;
		short *tmrow = mapaddr + (((yscroll/8 + y) & tilemapheightmask) << tilemapwidthshift);
		int tmx = l / 8;
		
		int llow = -(GetHScroll(y * 8 + 7, layer));
		float leftxmaplow = -(llow % 8);
		for(x = 0; x <= (screenwidthintiles+0); x++) {
			mapsample = tmrow[tmx++ & tilemapwidthmask];
			DrawTile32PVR(mapsample, sq, -1, leftxmap+xf, leftxmaplow+xf, yf, lowpriodepth, hipriodepth, TS, TS);
			
			xf += TS;
		}
		yf += TS;
	}
}


void DrawLayerCellPVR() {
	int x, y;
	float yf = 0, xf = 0;
	
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int mapsample;
	
	short *amapaddr = (short*)ScrA_Addr;
	short *bmapaddr = (short*)ScrB_Addr;
	
	for(y = 0; y < 32; y++) {
		xf = 0;
		for(x = 0; x < 64; x++) {
			//int mapsample = GetVdpVramWord(VdpMapAAddr+ y/8 * 64 + x*2)
			//int mapsample = VdpVramS[(VdpMapAAddr + y * 64 + x*2)^1];
			//mapsample = sh4SwapB(mapaddr[x + y*64]);
			mapsample = (amapaddr[x + y*64]);
			DrawTile32PVR(mapsample, sq, -1, xf, xf, yf, DEPTH_LAYER_A_LO, DEPTH_LAYER_A_HI, TS, TS);
			
			mapsample = (bmapaddr[x + y*64]);
			DrawTile32PVR(mapsample, sq, -1, xf, xf, yf, DEPTH_LAYER_B_LO, DEPTH_LAYER_B_HI, TS, TS);
			
			xf += TS;
		}
		yf += TS;
	}
}

static void DrawSprite(VDP_Sprite *spr, void *sq, float depth) {
	const int h = spr->flags & 0x3;
	const int w = (spr->flags >> 2) & 0x3;
	
	int x = spr->x & 0x1ff;// - 128;// + screen_start_x;
	int y = spr->y;// - 128;// + SCREEN_START_Y;
	if (VDP_Reg.Set4 & 0x6)
		y = (y & 0x3ff) / 2;
	else
		y = y & 0x1ff;
	int tile = spr->tile;
	int i, j;
	if (y < -80)
		return;
	if (x < -80)
		return;
	x*=TSCA;
	y*=TSCA;
	x += screen_start_x - 128;
	y += SCREEN_START_Y - 128;
	
	if (tile & VDP_VFLIP) {
		if (tile & VDP_HFLIP) {
			x -= TS;
			y -= TS;
			for(j = w+1; j; j--) {
				for(i = h+1; i; i--) {
					float scrx = x + j*TS;
					float scry = y + i*TS;
					DrawTile16Split(tile++, sq, -1, scrx, scrx, scry,
						DEPTH_LAYER_S_LO + depth, DEPTH_LAYER_S_HI + depth,
						TS, TS);
				}
			}
		} else {
			y -= TS;
			for(j = 0; j <= w; j++) {
				for(i = h+1; i; i--) {
					float scrx = x + j*TS;
					float scry = y + i*TS;
					DrawTile16Split(tile++, sq, -1, scrx, scrx, scry,
						DEPTH_LAYER_S_LO + depth, DEPTH_LAYER_S_HI + depth,
						TS, TS);
				}
			}
		}

	} else {
		if (tile & VDP_HFLIP) {
			x -= TS;
			for(j = w+1; j; j--) {
				for(i = 0; i <= h; i++) {
					float scrx = x + j*TS;
					float scry = y + i*TS;
					DrawTile16Split(tile++, sq, -1, scrx, scrx, scry,
						DEPTH_LAYER_S_LO + depth, DEPTH_LAYER_S_HI + depth,
						TS, TS);
				}
			}
		} else {
			for(j = 0; j <= w; j++) {
				for(i = 0; i <= h; i++) {
					float scrx = x + j*TS;
					float scry = y + i*TS;
					DrawTile16Split(tile++, sq, -1, scrx, scrx, scry,
						DEPTH_LAYER_S_LO + depth, DEPTH_LAYER_S_HI + depth,
						TS, TS);
				}
			}
		}
	}
}
void * draw_rect_wh(float x, float y, float w, float h, float z, unsigned int argb, void *dst) {
	return draw_rect(x, y, x+w, y+h, z, argb, dst);
}
void DrawSprites() {
	ColorSplitScreen = Raster_ColorSplitLine + SCREEN_START_Y;
	
	VDP_Sprite *sprlist = (VDP_Sprite*)Spr_Addr;
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	float depth = .90;
	int i;
	extern S32 H_Cell;	//screen width in tiles
	int maxsprites = H_Cell == 32 ? 64 : 80;
	VDP_Sprite *spr = sprlist;
	for(i = 0; i < maxsprites; i++) {
		DrawSprite(spr, sq, depth);
		depth -= 0.01f;
		spr = sprlist + spr->link;
	}
}

static inline void DrawModTile(void *sq, float topx, float topy, float bottomx, float bottomy, float z, float w) {
	pvr_vertex64s *v = sq;
	
	pv_set_cmd_vertex_eos(v);
	pv_set_pos0(v, topx, topy, z);
	pv_set_pos1(v, bottomx+w, bottomy, z);
	pv_set_submit_pos2(v, topx+w, topy, z);
	sqSubmit64B(sq);
	pv_set_submit_pos2(v, bottomx, bottomy, z);
	sqSubmit64B(sq);
}

void DrawMapRowMod(short *tmrow, int layer, int line, float scry, float z) {
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int tilemapwidthmask = H_Scroll_CMask;
	int tilemapwidthshift = H_Scroll_CMul;
	int mapsample;
	
	int screenwidthintiles = H_Cell+1;
	int l = -GetHScroll(line < 0 ? 0 : line, layer);
	int llow = l;
	
	int tmx, cols;
	int slope = signext(l - llow, tilemapwidthmask);
	float leftxmap, leftxmaplow;
	
	tmx = llow / 8;
	cols = screenwidthintiles + slope/8;
	leftxmaplow = screen_start_x - (llow % 8);
	leftxmap = leftxmaplow - slope;
	
	tmx--;
	leftxmap *= TSCA;
	leftxmaplow *= TSCA;
	leftxmap -= TS;
	leftxmaplow -= TS;
	
	int x;
	float modlen = 0, modstart = leftxmap, modstartlow = leftxmaplow;
	for(x = 0; x <= cols; x++) {
		mapsample = tmrow[tmx++ & tilemapwidthmask];
		//~ if (mapsample < 0) {
		if (mapsample & VDP_PRIO) {
			//Tile is high priority
			modlen++;
		} else {
			//Tile is low priority
			if (modlen) {
				DrawModTile(sq, modstart, scry, modstartlow, scry+TS, z, modlen * TW);
				//~ printf("%i ", modlen);
				modlen = 0;
			}
			modstart = leftxmap + TW;
			modstartlow = leftxmaplow + TW;
		}
		//~ DrawTile16PreRaster(mapsample, sq, -1, leftxmap, leftxmaplow, scry, lowpriodepth, hipriodepth, TS, TS);
		
		leftxmap += TS;
		leftxmaplow += TS;
	}
	
	if (modlen) {
		DrawModTile(sq, modstart, scry, modstartlow, scry+TS, z, modlen * TW);
		//~ printf("%i ", modlen);
	}
}

void DrawMapMod(int layer) {
	//TODO window support
	int x, y;
	float yf = SCREEN_START_Y, xf;
	
	int tilemapwidthshift = H_Scroll_CMul;
	int tilemapheightmask = V_Scroll_CMask; //tilemapheight-1;
	
	if (VDP_Reg.Win_V_Pos == 0x80 && layer == 0) {
		return;
	}
	
	int screenwidthintiles = H_Cell;
	int screenheightintiles = 28;
	
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int mapsample;
	
	short *mapaddr = layer ? (short*)ScrB_Addr : (short*)ScrA_Addr;
	int yscroll = vsram[layer] & 0x3ff;
	//yf = -(yscroll & 0x7);
	yf = SCREEN_START_Y - (yscroll % 8);
	yf *= TSCA;
	int yscrollofs = - (yscroll % 8);
	
	for(y = 0; y <= screenheightintiles; y++) {
		xf = screen_start_x;
		short *tmrow = mapaddr + (((yscroll/8 + y) & tilemapheightmask) << tilemapwidthshift);
		
		DrawMapRowMod(tmrow, layer, y*8, yf, DEPTH_LAYER_MOD_B);
		
		//~ DrawMapRow_NoLineSroll(tmrow, layer, y*8, yf, lowpriodepth, hipriodepth);
		yf += TS;
	}
}

int Enable_Shadow_Highlight = 1;

void DrawMapSH() {
	if (!Enable_Shadow_Highlight)
		return;
	
	//Do nothing if S/H isn't enabled
	if (!(VDP_Reg.Set4 & 0x08))
		return;
	
	pvr_context inc, exc;
	pc_set_default_polygon(&inc);
	pc_set_list(&inc, PC_OPAQUE_MOD);
	pc_set_mod_mode(&inc, PC_MOD_INCLUDE);
	pc_set_cull_mode(&inc, PC_CULL_DISABLE);
	inc.cmd |= 0x40;
	
	pc_copy(&inc, &exc);
	pc_set_mod_mode(&exc, PC_MOD_EXCLUDE);
	
	//~ printf("%08x ", inc.cmd);
	//~ printf("%08x\n", inc.mode1);
	//~ fflush(stdout);
	
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	
	//Put screen in shadow
	sqCopy32(sq, &inc);
	//~ DrawModTile(sq, screen_start_x, SCREEN_START_Y, screen_start_x, SCREEN_START_Y+224*TSCA, 320*TSCA, (DEPTH_LAYER_A_HI + DEPTH_LAYER_S_HI) / 2);
	DrawModTile(sq, 0, 0, 0, 240, DEPTH_LAYER_MOD_A, 320);
	
	sqCopy32(sq, &exc);
	DrawMapMod(0);
	DrawMapMod(1);
	//~ printf("\n");
	//~ fflush(stdout);
	//~ DrawMapMod(1);
	
}

