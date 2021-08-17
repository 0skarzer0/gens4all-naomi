#include <kos.h>
#include "pvr_rend.h"



PVRVRAMData __attribute__ ((aligned(32))) VdpTiles;
#ifdef PVRVDP_DBL_BUFF
pvr_context PVRTilesCxtBnk[2][VDP_PALETTE_CNT];
pvr_context PVRTilesSprCxtBnk[2][VDP_PALETTE_CNT];
int CurPVRBuff;
pvr_context *CurPVRTilesCxt;
pvr_context *CurPVRTilesSprCxt;
pvr_ptr_t PVRTilesTexture[2];
#else
pvr_context PVRTilesCxt[VDP_PALETTE_CNT];
pvr_context PVRTilesSprCxt[VDP_PALETTE_CNT];
//pvr_ptr_t PVRTilesTexture[2];
pvr_ptr_t PVRTilesTexture;
#endif

PvrUV32 __attribute__ ((aligned(32))) TileUV32Table[TILE_DEF_COMBOS];
PvrUV16 __attribute__ ((aligned(32))) TileUV16Table[TILE_DEF_COMBOS];
#define swap(a, b) do { typeof(a) _b = b; b = a; a = _b; } while(0)
void InitTileUVTable() {
	int i;
	float tilewidth = TILEWIDTHF / TEXWIDTHF;
	float tileheight = TILEHEIGHTF / TEXPVRHEIGHTF;
	
	for(i = 0; i < TILE_DEF_COMBOS; i++) {
		int fliph = i & VDP_HFLIP, flipv = i & VDP_VFLIP;
		int pal = VDP_GET_PAL(i);
		int tile = i & VDP_TILEMASK;
		int x = tile % TEXTILECOLS;
		int y = tile / TEXTILECOLS;
		
		y += 3 - pal;
		float left = x * tilewidth;
		float top = y * tileheight;
		float right = left + tilewidth;
		float bottom = top + tileheight;
		if (fliph)
			swap(left, right);
		if (flipv)
			swap(top, bottom);
		
		TileUV32Table[i].left = left;
		TileUV32Table[i].top = top;
		TileUV32Table[i].right = right;
		TileUV32Table[i].bottom = bottom;
		TileUV16Table[i].topleft = PVR_PACK_16BIT_UV(left, top);
		TileUV16Table[i].bottomright = PVR_PACK_16BIT_UV(right, bottom);
	}
}

void InitPVRTexture() {
	int i, j;
#ifdef PVRVDP_DBL_BUFF
	for(j = 0; j < 2; j++) {
		PVRTilesTexture[j] = pvr_mem_malloc(sizeof(PVRVRAMData));
		pvr_context *curcxt = PVRTilesCxtBnk[j];
		pvr_context *cursprcxt = PVRTilesSprCxtBnk[j];
		for(i = 0; i < VDP_PALETTE_CNT; i++) {
			
			//tri strip
			pc_set_default_polygon(curcxt+i);
			pc_set_gouraud(curcxt+i, 0);
			pc_set_list(curcxt+i, PVRVDP_TARGET_LIST);
			pc_set_color_format(curcxt+i, PC_PACKED);
			pc_set_blend_modes(curcxt+i, PC_SRC_ALPHA, PC_INV_SRC_ALPHA);
			//pc_set_blend_modes(curcxt+i, PC_ONE, PC_ZERO);
			pc_set_texture(curcxt+i, PVRTilesTexture[j] + 2048*i, PC_SIZE_1024, PC_SIZE_512, 
				PC_ARGB4444, 0, 0, 1);
			
			//sprites
			pc_copy(curcxt+i, cursprcxt+i);
			pc_set_command(cursprcxt+i, PC_CMD_SPRITE);
			pc_set_small_uv(cursprcxt+i, 1);
			pc_set_sprite_color_packed(cursprcxt+i, -1);
		}
	}
#else
	//for(j = 0; j < 2; j++) {
		//PVRTilesTexture[j] = pvr_mem_malloc(sizeof(PVRVRAMData));
		PVRTilesTexture = pvr_mem_malloc(sizeof(PVRVRAMData));
		for(i = 0; i < VDP_PALETTE_CNT; i++) {
			//tri strip
			pc_set_default_polygon(PVRTilesCxt+i);
			pc_set_gouraud(PVRTilesCxt+i, 0);
			pc_set_color_format(PVRTilesCxt+i, PC_PACKED);
			pc_set_blend_modes(PVRTilesCxt+i, PC_SRC_ALPHA, PC_INV_SRC_ALPHA);
			//pc_set_blend_modes(PVRTilesCxt+i, PC_ONE, PC_ZERO);
			//pc_set_texture(PVRTilesCxt+i, PVRTilesTexture[j] + 2048*i, PC_SIZE_1024, PC_SIZE_512, 
			pc_set_texture(PVRTilesCxt+i, PVRTilesTexture + 2048*i, PC_SIZE_1024, PC_SIZE_512, 
				PC_ARGB4444, 0, 0, 1);
			
			//sprites
			pc_copy(PVRTilesCxt+i, PVRTilesSprCxt+i);
			pc_set_command(PVRTilesSprCxt+i, PC_CMD_SPRITE);
			pc_set_small_uv(PVRTilesSprCxt+i, 1);
			pc_set_sprite_color_packed(PVRTilesSprCxt+i, -1);
		}
	//}
#endif
}

