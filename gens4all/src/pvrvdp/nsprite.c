

static void DRAW_SPRITE_NAME(VDP_Sprite *spr, void *sq, float depth, float topclip, float bottomclip) {
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
	x += screen_start_x - 128;
	y += SCREEN_START_Y - 128;
	x*=TSW;
	y*=TSH;
	
	//~ if (y > bottomclip)
		//~ return;
	
	if (tile & VDP_VFLIP) {
		if (tile & VDP_HFLIP) {
			x -= TW;
			y -= TH;
			for(j = w+1; j; j--) {
				for(i = h+1; i; i--) {
					float scrx = x + j*TW;
					float scry = y + i*TH;
					TILE_DRAW(tile++, sq, scrx, scry, depth,
						TW, TH);
				}
			}
		} else {
			y -= TH;
			for(j = 0; j <= w; j++) {
				for(i = h+1; i; i--) {
					float scrx = x + j*TW;
					float scry = y + i*TH;
					TILE_DRAW(tile++, sq, scrx, scry, depth,
						TW, TH);
				}
			}
		}

	} else {
		if (tile & VDP_HFLIP) {
			x -= TW;
			for(j = w+1; j; j--) {
				for(i = 0; i <= h; i++) {
					float scrx = x + j*TW;
					float scry = y + i*TH;
					TILE_DRAW(tile++, sq, scrx, scry, depth,
						TW, TH);
				}
			}
		} else {
			for(j = 0; j <= w; j++) {
				for(i = 0; i <= h; i++) {
					float scrx = x + j*TW;
					float scry = y + i*TH;
					TILE_DRAW(tile++, sq, scrx, scry, depth,
						TW, TH);
				}
			}
		}
	}
}
void DRAW_SPRITES_NAME() {
	ColorSplitScreen = Raster_ColorSplitLine + SCREEN_START_Y;
	//~ ColorSplitScreen = 1000;// + SCREEN_START_Y;
	
	VDP_Sprite *sprlist = Raster_Sprites[0];
	void *sq = sqPrepare((void*)PVR_TA_INPUT);
	float depth = .90;
	int i;
	extern S32 H_Cell;	//screen width in tiles
	int maxsprites = H_Cell == 32 ? 64 : 80;
	VDP_Sprite *spr = sprlist;
	for(i = 0; i < maxsprites; i++) {
		DRAW_SPRITE_NAME(spr, sq, depth, SCREEN_START_Y, Raster_SpriteSplitLine + SCREEN_START_Y);
		depth -= 0.01f;
		if (spr->link > maxsprites)
			break;
		spr = sprlist + spr->link;
	}
	
	if (1 && Raster_SpriteSplitLine < 224) {
		sprlist = Raster_Sprites[1];
		VDP_Sprite *spr = sprlist;
		depth = .90;
		for(i = 0; i < maxsprites; i++) {
			DRAW_SPRITE_NAME(spr, sq, depth, Raster_SpriteSplitLine + SCREEN_START_Y, 224+SCREEN_START_Y);
			depth -= 0.01f;
			if (spr->link > maxsprites)
				break;
			spr = sprlist + spr->link;
		}
	}
}
