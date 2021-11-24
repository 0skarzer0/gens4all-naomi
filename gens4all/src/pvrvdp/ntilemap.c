

#ifdef MY_DRAW_CODE
#if R_DECODE_LAYER(MY_DRAW_CODE) == R_MAP_A
	#define LAYER	0
	#define MY_VSCROLL	vscroll_a
	#define MY_PAT_SCR_ADR	(((Raster_Data[i].regs.Pat_ScrA_Adr) & 0x38) << 10)
	#if R_DECODE_WINDOW(MY_DRAW_CODE)
		#define WINDOWED 1
		#define MAP_CHANGES (RASTER_LINE_WIN_CHANGE|RASTER_LINE_MAPA_CHANGE)
	#else
		#define WINDOWED 0
		#define MAP_CHANGES (RASTER_LINE_MAPA_CHANGE)
	#endif
	#define HI_DEPTH	DEPTH_LAYER_A_HI
	#define LO_DEPTH	DEPTH_LAYER_A_LO
#else
	#define LAYER	1
	#define MY_VSCROLL	vscroll_b
	#define MY_PAT_SCR_ADR	(((Raster_Data[i].regs.Pat_ScrB_Adr) & 0x7) << 13)
	#define WINDOWED 0
	#define MAP_CHANGES (RASTER_LINE_MAPB_CHANGE)
	#define HI_DEPTH	DEPTH_LAYER_B_HI
	#define LO_DEPTH	DEPTH_LAYER_B_LO
#endif

#define RASTER_LVL R_DECODE_RASTER(MY_DRAW_CODE)
#define AUTO_SPLIT R_DECODE_SPLITS(MY_DRAW_CODE)
#define CELL_TILT R_DECODE_TILT(MY_DRAW_CODE)

#endif
//*/
void DRAW_MAP_FUNC() {
	int tilemapwidthmask = H_Scroll_CMask;
	int tilemapwidthshift = H_Scroll_CMul;
	int tilemapheightmask = V_Scroll_CMask;
	
	int screenwidthintiles = H_Cell;
	extern int inkey;
	
	//~ UpdateWindow(&Raster_Data[i].regs);
	
	int i = 0, j;
	int vscrollsplits;
	int linestonexttile;
	int curpal = 0;
	//~ RowSplits splitdata[screenheightinpixels / 8];
	int wintilemapwidthshift = (H_Cell == 32) ? 5 : 6;
	VDPDrawMode drawmode = {CurPVRTilesCxt, CurPVRTilesSprCxt, 0, H_Scroll_CMask, HI_DEPTH, LO_DEPTH, 0, 1, TW};
#if WINDOWED
	VDPDrawMode drawmodewin = {CurPVRTilesCxt, CurPVRTilesSprCxt, 0, (H_Cell == 32) ? 31 : 63 , HI_DEPTH+0.1, LO_DEPTH+0.1, 0, 1, TW};
	if (inkey == 'w') {
		printf("L: %i R: %i\n", WindowData.left, WindowData.right);
		printf("T: %i B: %i\n", WindowData.top, WindowData.bottom);
	}
#endif
	
	Raster_Lines[screenheightinpixels] |= RASTER_LINE_SCREEN_OFF;
	
	
	U32 MapAddr = MY_PAT_SCR_ADR + (U32)VRam;
	
	int last_remaining = 8;
	int yscroll = Raster_Data[0].MY_VSCROLL;
	sqSetTargetAddress((void*)PVR_TA_INPUT);
	
	//~ float ypos = SCREEN_START_Y - ((Raster_Data[0].MY_VSCROLL & 0x3ff) % 8);
	for(i = 0; i < screenheightinpixels; ) {
		if (LINE_BLANKED(i)) {
			i++;
			continue;
		}
		int right = VDP_STRETCH_256 ? 640 : 320;
		
		
		if (i >= Raster_ColorSplitLine) {
			drawmode.tileofs = 0x10000;
			drawmode.tex = CurPVRTilesCxtAlt;
			drawmode.texspr = CurPVRTilesSprCxtAlt;
#if WINDOWED
			drawmodewin.tileofs = 0x10000;
			drawmodewin.tex = CurPVRTilesCxtAlt;
			drawmodewin.texspr = CurPVRTilesSprCxtAlt;
#endif
		}
		
#if WINDOWED
		UpdateWindow(&Raster_Data[i].regs);
		if (i < WindowData.top || i >= WindowData.bottom) {
#endif
			//This row is not fully replaced by window
			int l = -GetHScroll(i, LAYER);
#if RASTER_LVL >= 2
			yscroll = Raster_Data[i].MY_VSCROLL;
			if (VDP_Reg.Set4 & 0x6)
				yscroll /= 2;
#endif
			
			//~ int tileyofs = (yscroll % 8);
			int map_row = (yscroll+i)/8;
			int offset_from_tile_top = ((yscroll + i) & 0x7);
			int remaining_tile = 8 - offset_from_tile_top;
			
			float nypos = SCREEN_START_Y + i;
			
			int j, have_raster = 0, first_raster = -1, last_raster = 0;
			int yscrolldelta = 0;
			int stretchmode = 0;
			int tile_height = remaining_tile;
			int remaining_raster_scan;
			int flip = 0;
			
			drawmode.topsplit = offset_from_tile_top / 8.0f;
#if RASTER_LVL >= 2
			//Check for stretched final line of tile
			if (remaining_tile == 1) {
				if (i ==1) {
					//~ printf("1YS: %i, NL: %i\n", yscroll, Raster_Data[i+1].MY_VSCROLL+1);
					//~ printf("2YS: %i, NL: %i\n", yscroll, Raster_Data[i+2].MY_VSCROLL+2);
				}
				for(j = 1; j < (screenheightinpixels-i); j++) {
					if ((Raster_Data[i+j].MY_VSCROLL + j) == (yscroll)) {
						tile_height++;
					} else {
						break;
					}
				}
				if (j != 1) {
					//~ printf("3YS\n");
					drawmode.lowsplit = drawmode.topsplit + 1/8.0;
					goto checkpal;
				}
			}
			
			for(j = 1, remaining_raster_scan = remaining_tile; j < remaining_raster_scan; j++) {
				if (Raster_Lines[i+j] & (MAP_CHANGES|RASTER_LINE_SCREEN_OFF)) {
					remaining_tile = j;
					tile_height = j;
					goto exitraster;
				}
				int curscroll = Raster_Data[i+j].MY_VSCROLL;
				if (1 && curscroll != yscroll) {
					
					have_raster++;
					last_raster = j;
					if (first_raster < 0)
						first_raster = j;
/*
					if ((yscroll+j) == curscroll) {
						//Stretching single line
						if ((j == 1) && !stretchmode) {
							//enter stretch mode
							stretchmode = 1;
							//printf("S");
						} else {
							//printf("s");
						}
						draw_rect_wh(64, SCREEN_START_Y+i+j, 8, 1, 15, -1, sqPrepare((void*)PVR_TA_INPUT));
						remaining_raster_scan++;
					} else {
						if (stretchmode) {
							//printf(".");
							tile_height = j;
							drawmode.lowsplit = drawmode.topsplit = offset_from_tile_top / 8.0f;
							goto checkpal;
						} else if (yscroll > curscroll) { 
							//Stretch more than one line
						} else if (yscroll < curscroll) {
							//Compress/Upsidedown
							yscrolldelta--;
							//flip = 1;
						} else {
							
						}
					}
*/
					
				}
			}
			#ifdef SONIC3DBLASTFMV
			if (1 && have_raster) {
				if (have_raster == 1) {
					remaining_tile = first_raster;
					//~ printf("1"); fflush(stdout);
				} else {
					remaining_tile = (first_raster + last_raster) / 2;
					//~ remaining_tile = last_raster;
				}
				//~ tile_height = remaining_tile;
				
			}
			#else
			if (1 && have_raster) {
				if (have_raster == 1) {
					remaining_tile = first_raster;
					//~ printf("1"); fflush(stdout);
				} else {
					//~ remaining_tile = (first_raster + last_raster) / 2;
					remaining_tile = last_raster;
				}
				tile_height = remaining_tile;
				
			}
			#endif
#endif
		exitraster:
#if AUTO_SPLIT
			if ((i+remaining_tile) > 224)	remaining_tile = 224 - i;
			if (!have_raster && remaining_tile > AUTO_SPLIT) {
				int first_split = 0, splits = 0, s = l;
				for(j = 1; j < remaining_tile; j++) {
					int s2 = -GetHScroll(i+j, LAYER);
					if (s != s2) {
						splits++;
						s = s2;
						if (first_split == 0) {
							first_split = j;
						}
					}
				}
				if (splits == 0) {
					
				} else if (splits == 1) {
					tile_height = remaining_tile = first_split;
				} else if (tile_height > AUTO_SPLIT) {
					tile_height = remaining_tile = AUTO_SPLIT;
				}
			}
#endif
				//~ if (tile_height > 4) { tile_height = 4; remaining_tile = 4; }
			//~ if (LAYER == 2) {
				//~ if (tile_height > 2) { tile_height = 2; remaining_tile = 2; }
			//~ }
			
		
			//~ drawmode.topsplit = offset_from_tile_top / 8.0f;
			drawmode.lowsplit = (offset_from_tile_top + remaining_tile) / 8.0f;
#if RASTER_LVL >= 1
		checkpal:
			//Check for waterline raster
			if (1 && (i < Raster_ColorSplitLine) && ((i + tile_height) > Raster_ColorSplitLine)) {
				//~ float percentage = (float)(Raster_ColorSplitLine - i) / (float)(i + tile_height - Raster_ColorSplitLine);
				float percentage = (float)(Raster_ColorSplitLine - i) / (float)(tile_height );
				//~ printf("L: %i\nCSL %i\nTH: %i\n%f percent\n", i, (int)Raster_ColorSplitLine, tile_height, percentage);
				
				drawmode.lowsplit = drawmode.topsplit + (drawmode.lowsplit - drawmode.topsplit) * percentage;
				//~ remaining_tile = Raster_ColorSplitLine - i;
				tile_height = Raster_ColorSplitLine - i;
			}
#endif
			
			//~ tile_height = remaining_tile;
#if RASTER_LVL >= 2
			MapAddr = MY_PAT_SCR_ADR + (U32)VRam;
#endif
			//~ short *tmrow = (short*)MapAddr + ((((Raster_Data[i].MY_VSCROLL & 0x3ff) + i)/ 8 & V_Scroll_CMask) << H_Scroll_CMul);
			short *tmrow = (short*)MapAddr + ((map_row & V_Scroll_CMask) << H_Scroll_CMul);
#if CELL_TILT
			
			int llow = -GetHScroll((i + tile_height-1) >= 223 ? 223 : (i + tile_height-1), LAYER);
			int tilecount, tmx;
			int slope = signext(l - llow, tilemapwidthmask);
			float leftxmap, leftxmaplow;
			if (slope < 0) {
				//top line is to the left of bottom line
				tmx = l / 8;
				tilecount = screenwidthintiles - slope/8 + 2;
				leftxmap = screen_start_x - (l % 8);
				leftxmaplow = leftxmap + slope;
			} else {
				tmx = llow / 8;
				tilecount = screenwidthintiles + slope/8 + 2;
				leftxmaplow = screen_start_x - (llow % 8);
				leftxmap = leftxmaplow - slope;
			}
			
			//~ tilecount++;
			leftxmap *= TSW;
			leftxmaplow *= TSW;
			tmx--;
			leftxmap -= TW;
			leftxmaplow -= TW;
#else
			//~ int l = -GetHScroll(i, LAYER);
			int tmx = l / 8;
			int tilecount = screenwidthintiles + 1;
			
			float leftxmap = screen_start_x - (l % 8);
			tmx--;
			leftxmap *= TSW;
			leftxmap -= TW;
			float leftxmaplow = leftxmap;
			//~ float leftxmap = TSW*(screen_start_x - (l % 8)) - TW;
			//~ float leftxmaplow = leftxmap;
			
#endif
			if (0 && i == 0 && WINDOWED) {
				printf("TC: %i\n", tilecount);
				fflush(stdout);
			}
			//~ if (flip) {
				//~ nypos += tile_height;
				//~ tile_height = -tile_height;
			//~ }
#if WINDOWED
			//TODO This method of testing the window region doesn't seem right, but it seems to work
			if (WindowData.right > 0) {
				//Part of this row is a window
				if (WindowData.left > 0) {
					//Window on right
					//TODO is this right? Helps split in Herzog Zwei (but it's not perfect).
					//Doing it unconditionally affects Zombie Ate My Heighbors radar
					//~ if (l & 0xf)
						//~ WindowData.left += 16;
					tilecount -= H_Cell - WindowData.left/8;
					DrawMapRowClipped3(&drawmode, tmrow, tmx, tilecount, leftxmap, nypos, leftxmaplow, nypos+tile_height);
					
//					U32 MapAddr = (((Raster_Data[i].regs.Pat_Win_Adr) & 0x3C) << 10) + (U32)VRam;
//					short *tmrow = (short*)MapAddr + ((i/8) << wintilemapwidthshift);
					//~ DrawMapRowUnclipped(&drawmodewin, tmrow, WindowData.left, screenwidthintiles - WindowData.left/8, WindowData.left+screen_start_x, (i&0x7)+SCREEN_START_Y, WindowData.left+screen_start_x, (i&0x7)+SCREEN_START_Y+remaining_tile);
//					DrawMapRowClipped3(&drawmodewin, tmrow, WindowData.left/8, screenwidthintiles - WindowData.left/8, WindowData.left+screen_start_x, (i&~0x7)+SCREEN_START_Y, WindowData.left+screen_start_x, (i&~0x7)+SCREEN_START_Y+tile_height);
				} else {
					//Window on left
					leftxmap += WindowData.right;
					leftxmaplow += WindowData.right;
					tmx += WindowData.right/8;
					tilecount -= WindowData.right/8;
					DrawMapRowClipped3(&drawmode, tmrow, tmx, tilecount, leftxmap, nypos, leftxmaplow, nypos+tile_height);
					
//					U32 MapAddr = (((Raster_Data[i].regs.Pat_Win_Adr) & 0x3C) << 10) + (U32)VRam;
//					short *tmrow = (short*)MapAddr + ((i/8) << wintilemapwidthshift);
//					DrawMapRowClipped3(&drawmodewin, tmrow, 0, WindowData.right/8, screen_start_x, i+SCREEN_START_Y, screen_start_x, i+SCREEN_START_Y+tile_height);
				}
//				draw_rect_wh(320-24, SCREEN_START_Y+i, 8, 1, 15, -1, sqPrepare((void*)PVR_TA_INPUT));
			} else {
#endif
				//This row has no window at all
				DrawMapRowClipped3(&drawmode, tmrow, tmx, tilecount, leftxmap, nypos, leftxmaplow, nypos+tile_height);
#if WINDOWED
			}
#endif
			if (Show_Rasters)
	#if LAYER == R_MAP_A
				draw_rect_wh(right-8, SCREEN_START_Y+i, 8, 1, 15, -1, sqPrepare((void*)PVR_TA_INPUT));
	#else
				draw_rect_wh(right-16, SCREEN_START_Y+i, 8, 1, 15, 0xff0000ff, sqPrepare((void*)PVR_TA_INPUT));
	#endif
			//~ if (flip) {
				//~ i -= tile_height;
			//~ } else {
				//~ i += tile_height;
			//~ }
			i += tile_height;
#if WINDOWED
		} else {
//			if (Show_Rasters)
//				draw_rect_wh(320-24, SCREEN_START_Y+i, 8, 1, 15, -1, sqPrepare((void*)PVR_TA_INPUT));
			//This row is a pure window row
			U32 MapAddr = (((Raster_Data[i].regs.Pat_Win_Adr) & 0x3C) << 10) + (U32)VRam;
			short *tmrow = (short*)MapAddr + ((i/8) << wintilemapwidthshift);
			
//			DrawMapRowUnclipped(&drawmodewin, tmrow, 0, screenwidthintiles, (i&~0x7)+SCREEN_START_Y, screen_start_x, screen_start_x);
			i += 8;
			
		}
#endif
		
	}
}

#undef WINDOWED
#undef MY_VSCROLL
#undef MY_PAT_SCR_ADR
#undef MAP_CHANGES
#undef DRAW_MAP_FUNC
#undef HI_DEPTH
#undef LO_DEPTH
#undef LAYER
#undef MY_DRAW_CODE

#undef RASTER_LVL
#undef CELL_TILT
#undef AUTO_SPLIT
