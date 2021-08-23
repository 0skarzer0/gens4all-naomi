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

#define DrawTile32PVR(a...) DrawTile16PVR_C(a)
//~ #define DrawTile32PVR(a...) DrawTile32PVR_Asm(a)

extern struct Reg_VDP_Type VDP_Reg;

float screen_start_x = 0;

void DrawTile32PVR_C(int tile, void *sq, int color, float topx, float bottomx, float y, float lowz, float hiz, float w, float h) {
	PvrUV32 *uv = TileUV32Table + (tile & 0xffff);
	
#ifdef PVRVDP_DBL_BUFF
	sq = sqCopy32(sq, &CurPVRTilesCxt[VDP_GET_PAL(tile)]);
#else
	sq = sqCopy32(sq, &PVRTilesCxt[VDP_GET_PAL(tile)]);
#endif
	
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

void DrawTile16PVR_C(int tile, void *sq, int color, float topx, float bottomx, float y, float lowz, float hiz, float w, float h) {
	PvrUV16 *uv = TileUV16Table + (tile & 0xffff);
#ifdef PVRVDP_DBL_BUFF
	sq = sqCopy32(sq, &CurPVRTilesSprCxt[VDP_GET_PAL(tile)]);
#else
	sq = sqCopy32(sq, &PVRTilesSprCxt[VDP_GET_PAL(tile)]);
#endif
	
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
void * draw_rect(float x0, float y0, float x1, float y1, float z, unsigned int argb, void *dst) {
	pvr_context spr;
	pc_set_default_sprite(&spr);
	pc_set_sprite_color_packed(&spr, argb);
	pc_set_list(&spr, PVRVDP_TARGET_LIST);
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
		dst[i*4 + 0] = colors[i >> 4];
		dst[i*4 + 1] = colors[i >> 4];
		dst[i*4 + 2] = colors[i & 0xf];
		dst[i*4 + 3] = colors[i & 0xf];
	}
}
int testcolor = 0;
void DrawVRAMPVRManual(float scrx, float scry, float depth) {
	int y, x;
	float yf = scry, xf = scrx;
	void *pc = sqPrepare((void*)PVR_TA_INPUT);
	for(y = 0; y < 32; y++) {
		xf = scrx;
		for(x = 0; x < 64; x++) {
			//DrawTile32PVR_C((y*64+x%64) | (testcolor<<(VDP_PALSHIFT-2)), pc, -1, xf, xf, yf, 0.5, 0.5, 8, 8);
			DrawTile32PVR((y*64+x%64) | (testcolor<<(VDP_PALSHIFT-2)), pc, -1, xf, xf, yf, depth, depth, 8, 8);
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

extern unsigned int ScrA_Addr;
extern unsigned int ScrB_Addr;
extern unsigned int Spr_Addr;
extern unsigned char VSRam[256];

extern unsigned int Win_Addr;
extern unsigned int H_Win_Mul;
extern unsigned int Win_X_Pos;
extern unsigned int Win_Y_Pos;

short *vsram = (short*)&VSRam;


extern unsigned int H_Scroll_Addr;
extern unsigned int H_Scroll_Mask;
int GetHScroll(int line, int map) {
	short *hscrolltable = (short*)H_Scroll_Addr;
	return hscrolltable[(line & H_Scroll_Mask) * 2 + map];// & 0x3ff;
}
void DrawMapsPVR() {
	int x, y;
	float yfa = 0, xfa = 0;
	float yfb = 0, xfb = 0;
	
	
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	int mapsample;
	
	short *amapaddr = (short*)ScrA_Addr;
	short *bmapaddr = (short*)ScrB_Addr;
	int yscrolla = vsram[0];
	int yscrollb = vsram[1];
	yfa = -(yscrolla & 0x7);
	yfb = -(yscrollb & 0x7);
	
	for(y = 0; y < 29; y++) {
		xfa = 0;
		xfb = 0;
		int la = -GetHScroll(y * 8, 0) + xfa;
		int lb = -GetHScroll(y * 8, 1) + xfb;
		//int lb = GetHScroll(y * 8, 1);
		//int lat = la ;
		//int lbt = lb ;
		float leftxmapa = -(la & 7);
		float leftxmapb = -(lb & 7);
		//printf("LM: %f   ", leftxmapa);
		//float leftxmapb = lb;
		//int maplinestart = ((yp / 8) % 32) * 64;
		short *tmarow = amapaddr + (((yscrolla / 8) + y) % 32) * 64;
		short *tmbrow = bmapaddr + (((yscrollb / 8) + y) % 32) * 64;
		int tmxa = la / 8;
		int tmxb = lb / 8;
		for(x = 0; x < 41; x++) {
			//int mapsample = GetVdpVramWord(VdpMapAAddr+ y/8 * 64 + x*2)
			//int mapsample = VdpVramS[(VdpMapAAddr + y * 64 + x*2)^1];
			//mapsample = sh4SwapB(mapaddr[x + y*64]);
			//mapsample = (amapaddr[ ((lat+x)&0x63) + y*64]);
			//mapsample = (amapaddr[ ((x+&63) + y*64]);
			mapsample = tmarow[tmxa++ & 63];
			DrawTile32PVR(mapsample, sq, -1, leftxmapa+xfa, leftxmapa+xfa, yfa, DEPTH_LAYER_A_LO, DEPTH_LAYER_A_HI, TS, TS);
			
			//mapsample = (bmapaddr[ (x&0x63) + y*64]);
			mapsample = (tmbrow[tmxb++ & 63]);
			//DrawTile32PVR(mapsample, sq, -1, leftxmapb+xf, leftxmapb+xf, yf, DEPTH_LAYER_B_LO, DEPTH_LAYER_B_HI, TS, TS);
			DrawTile32PVR(mapsample, sq, -1, leftxmapb+xfb, leftxmapb+xfb, yfb, DEPTH_LAYER_B_LO, DEPTH_LAYER_B_HI, TS, TS);
			
			xfa += TS;
			xfb += TS;
		}
		yfa += TS;
		yfb += TS;
	}
}

void DrawMapPVR(int layer, float lowpriodepth, float hipriodepth) {
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
	
	if (!wvp)
		return;
	
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
typedef struct {
	short y;
	char link, flags;
	short tile;
	short x;
} VDPSprite;
void DrawSprite(VDPSprite *spr, void *sq, float depth) {
	const int h = spr->flags & 0x3;
	const int w = (spr->flags >> 2) & 0x3;
	int x = (spr->x & 0x1ff) - 128 + screen_start_x;
	int y = (spr->y & 0x1ff) - 128 + SCREEN_START_Y;
	int tile = spr->tile;
	int i, j;
	
	if (tile & VDP_VFLIP) {
		if (tile & VDP_HFLIP) {
			x -= TS;
			y -= TS;
			for(j = w+1; j; j--) {
				for(i = h+1; i; i--) {
					float scrx = x + j*TS;
					float scry = y + i*8;
					DrawTile32PVR(tile++, sq, -1, scrx, scrx, scry,
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
					DrawTile32PVR(tile++, sq, -1, scrx, scrx, scry,
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
					DrawTile32PVR(tile++, sq, -1, scrx, scrx, scry,
						DEPTH_LAYER_S_LO + depth, DEPTH_LAYER_S_HI + depth,
						TS, TS);
				}
			}
		} else {
			for(j = 0; j <= w; j++) {
				for(i = 0; i <= h; i++) {
					float scrx = x + j*TS;
					float scry = y + i*TS;
					DrawTile32PVR(tile++, sq, -1, scrx, scrx, scry,
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
	VDPSprite *sprlist = (VDPSprite*)Spr_Addr;
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	float depth = .90;
	int i;
	extern S32 H_Cell;	//screen width in tiles
	int maxsprites = H_Cell == 32 ? 64 : 80;
	VDPSprite *spr = sprlist;
	//TODO max 64 sprites fro 32 wide
	for(i = 0; i < maxsprites; i++) {
		DrawSprite(spr, sq, depth);
		depth -= 0.01f;
		spr = sprlist + spr->link;
	}
}
