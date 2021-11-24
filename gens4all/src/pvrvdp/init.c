#include <kos.h>
#include "pvr_rend.h"



pvr_context __attribute__ ((aligned(32))) PVRTilesCxtBnk[2][VDP_PALETTE_CNT];
pvr_context __attribute__ ((aligned(32))) PVRTilesCxtBnkAlt[2][VDP_PALETTE_CNT];
pvr_context __attribute__ ((aligned(32))) PVRTilesSprCxtBnk[2][VDP_PALETTE_CNT];
pvr_context __attribute__ ((aligned(32))) PVRTilesSprCxtBnkAlt[2][VDP_PALETTE_CNT];
int CurPVRBuff;
pvr_context *CurPVRTilesCxt;
pvr_context *CurPVRTilesCxtAlt;
pvr_context *CurPVRTilesSprCxt;
pvr_context *CurPVRTilesSprCxtAlt;
pvr_ptr_t PVRTilesTexture[2];

PvrUV32 __attribute__ ((aligned(32))) TileUV32Table[TILE_DEF_COMBOS*PVRVDP_CRAST_CNT];
PvrUV16 __attribute__ ((aligned(32))) TileUV16Table[TILE_DEF_COMBOS*PVRVDP_CRAST_CNT];
#define swap(a, b) do { typeof(a) _b = b; b = a; a = _b; } while(0)


pvr_ptr_t PVRBGColorTexture[2];
pvr_context __attribute__ ((aligned(32))) PVRBgColorCxt[2];


VDPGeometry vdpgeo;

void InitTileUVTable() {
	int i;
	float tilewidth = TILEWIDTHF / TEXWIDTHF;
	float tileheight = TILEHEIGHTF / TEXPVRHEIGHTF;
	
	for(i = 0; i < TILE_DEF_COMBOS*PVRVDP_CRAST_CNT; i++) {
		int fliph = i & VDP_HFLIP, flipv = i & VDP_VFLIP;
		int pal = VDP_GET_PAL(i);
		int tile = i & VDP_TILEMASK;
		int x = tile % TEXTILECOLS;
		int y = tile / TEXTILECOLS;
		
		if (i < TILE_DEF_COMBOS)
			y += 7 - pal;
		else
			y += 3 - pal;
		
		float left = x * tilewidth;
		float top = y * tileheight;
		float right = left + tilewidth;
		float bottom = top + tileheight;
		if (!fliph)
			swap(left, right);
		if (flipv)
			swap(top, bottom);
		
		TileUV32Table[i].left = left;
		TileUV32Table[i].right = right;
		TileUV32Table[i].left = left - ((fliph ? -0.3 : 0.25) / TEXPVRWIDTHF);
		TileUV32Table[i].right = right + ((fliph ? -0.3 : 0.25)  / TEXPVRWIDTHF);
		TileUV32Table[i].top = top;
		TileUV32Table[i].bottom = bottom;
		TileUV16Table[i].topleft = PVR_PACK_16BIT_UV(left, top);
		TileUV16Table[i].bottomright = PVR_PACK_16BIT_UV(right, bottom);
	}
}

void InitPVRTexture() {
	int i, j;
	for(j = 0; j < 2; j++) {
		PVRTilesTexture[j] = pvr_mem_malloc(sizeof(PVRVRAMData));
		
		pvr_context *curcxt = PVRTilesCxtBnk[j];
		pvr_context *curcxtalt = PVRTilesCxtBnkAlt[j];
		pvr_context *cursprcxt = PVRTilesSprCxtBnk[j];
		pvr_context *cursprcxtalt = PVRTilesSprCxtBnkAlt[j];
		for(i = 0; i < VDP_PALETTE_CNT; i++) {
			
			//tri strip
			pc_set_default_polygon(curcxt+i);
			pc_set_gouraud(curcxt+i, 0);
			pc_set_list(curcxt+i, PVRVDP_TARGET_LIST);
			pc_set_color_format(curcxt+i, PC_PACKED);
			pc_set_blend_modes(curcxt+i, PC_SRC_ALPHA, PC_INV_SRC_ALPHA);
			pc_set_cull_mode(curcxt+i, PC_CULL_DISABLE);
			pc_set_modified(curcxt+i, 1);
			pc_set_modifier_full(curcxt+i, 0);
			//pc_set_blend_modes(curcxt+i, PC_ONE, PC_ZERO);
			pc_set_texture(curcxt+i, PVRTilesTexture[j] + PVR_CODEBOOK_SIZE*i, PC_SIZE_1024, PC_SIZE_512, 
				PC_ARGB4444, 0, 0, 1);
			
			pc_copy(curcxt+i, curcxtalt+i);
			pc_set_texture(curcxtalt+i, PVRTilesTexture[j] + PVR_CODEBOOK_SIZE*i + PVR_CODEBOOK_SIZE*4, PC_SIZE_1024, PC_SIZE_512, 
				PC_ARGB4444, 0, 0, 1);
			
			//sprites
			pc_copy(curcxt+i, cursprcxt+i);
			pc_set_command(cursprcxt+i, PC_CMD_SPRITE);
			pc_set_small_uv(cursprcxt+i, 1);
			pc_set_sprite_color_packed(cursprcxt+i, -1);
			
			pc_copy(cursprcxt+i, cursprcxtalt+i);
			pc_set_texture(cursprcxtalt+i, PVRTilesTexture[j] + PVR_CODEBOOK_SIZE*i + PVR_CODEBOOK_SIZE*4, PC_SIZE_1024, PC_SIZE_512, 
				PC_ARGB4444, 0, 0, 1);
			
		}
		
		PVRBGColorTexture[j] = pvr_mem_malloc(256*4);
		pvr_context *bgcxt = PVRBgColorCxt + j;
		
		pc_set_default_sprite(bgcxt);
		pc_set_list(bgcxt, PC_OPAQUE_POLY);
		pc_set_color_format(bgcxt, PC_PACKED);
		pc_set_blend_modes(bgcxt, PC_ONE, PC_ZERO);
		pc_set_cull_mode(bgcxt, PC_CULL_DISABLE);
		pc_set_modified(bgcxt, 1);
		pc_set_modifier_full(bgcxt, 0);
		pc_set_texture(bgcxt, PVRBGColorTexture[j], PC_SIZE_512, PC_SIZE_8, 
			PC_ARGB4444, 0, 0, 0);
		pc_set_sprite_color_packed(bgcxt, -1);
		pc_set_disable_texture_alpha(bgcxt, 1);
	}
}

