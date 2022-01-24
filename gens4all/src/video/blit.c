
#include "gens4all.h"
#include "vdp_rend.h"
#include "blit.h"

#ifndef U32
	#define U32 unsigned int
#endif

#ifndef DREAMCAST
void Blit_X1(unsigned char *Dest, const int pitch, const int x, const int y, const int offset)
{
	int i;
	register U32 *s = (U32 *) MD_Screen + 4;
	register U32 *d = (U32 *)Dest;
	const U32 nx = (x >> 4);
	const U32 soff = (offset/4);
	const U32 doff = (pitch - x*2)/4;

	for (i = 0; i < y; i++)
	{
		register int n=nx;
		while(n--)
		{
			*d++ = *s++;
			*d++ = *s++;
			*d++ = *s++;
			*d++ = *s++;
			*d++ = *s++;
			*d++ = *s++;
			*d++ = *s++;
			*d++ = *s++;
		}
		s += soff;
		d += doff;
	}
}

void Blit_Scale2x (unsigned char *Dest, int pitch, int x, int y, int offset) 
{
	int i, j;
	int B, D, E, F, H;
	int E0, E1, E2, E3;
  
    // A B C
    // D E F
    // G H I
  
	int ScrWdth, ScrAdd;
	int SrcOffs, DstOffs;

	ScrAdd = offset >> 1;
	ScrWdth = x + ScrAdd;
	SrcOffs = 8;
	DstOffs = 0;
  
	for (i = 0; i < y; i++)
    {
		E = MD_Screen[SrcOffs - 1];
		F = MD_Screen[SrcOffs];
		DstOffs = i * pitch * 2;
		
		for (j = 0; j < x; j++)
		{
			D = E;
			E = F;
			B = MD_Screen[SrcOffs - ScrWdth];
			H = MD_Screen[SrcOffs + ScrWdth];
			F = MD_Screen[++SrcOffs];
			E0 = D == B && B != F && D != H ? D : E;
			E1 = B == F && B != D && F != H ? F : E;
			E2 = D == H && D != B && H != F ? D : E;
			E3 = H == F && D != H && B != F ? F : E;
	  
			// E0
			Dest[DstOffs] = E0;
			Dest[DstOffs + 1] = E0 >> 8;
	  
			// E1
			Dest[DstOffs + 2] = E1;
			Dest[DstOffs + 3] = E1 >> 8;
	  
			// E2
			Dest[DstOffs + pitch] = E2;
			Dest[DstOffs + pitch + 1] = E2 >> 8;
	  
			// E3
			Dest[DstOffs + pitch + 2] = E3;
			Dest[DstOffs + pitch + 3] = E3 >> 8;

			DstOffs += 4;
		}     

		SrcOffs += ScrAdd;
	}
}


#else

#if 1
#include "../../../libsh4/sh4.h"
#include "../../../libsh4/sq.h"
//#include "../pvrvdp/pvr_cxt.h"
#include "../pvrvdp/npvr_vertex.h"
#include "../pvrvdp/pvr_rend.h"



void PrintTimer() {
	const int sectowait = 10;
	static int tick = sectowait*60;
	static int sec = 0;
	if (--tick == 0) {
		tick = sectowait*60;
		sec += sectowait;
		printf("%i sec. ", sec);
		fflush(stdout);
	}
}

#include "vdp_io.h"


int Show_Timing = DEBUGDISPLAYDEFAULT;
int Show_Rasters = DEBUGDISPLAYDEFAULT;
int Show_FPS = DEBUGDISPLAYDEFAULT;

int Enable_Win = 1;
int Enable_MapA = 1;
int Enable_MapB = 1;
int Enable_Sprites = 1;

int FrameSkipper = 0;
int LastVBL = 0;
int LastVBLRender = 0;

extern int inkey;

#define VBLCNT (60)
char FrameVBLs[VBLCNT];
int CurFrameVBL;

int BlendSprites = 0;

void LockList(int list) {
	pvr_context spr;
	pc_set_default_polygon(&spr);
	pc_set_list(&spr, list);
	sqCopy32(sqPrepare((void*)PVR_TA_INPUT), &spr);
}

void Blit_Dreamcast(unsigned char *Dest, const int x, const int y, const int offset)
{
prof_start(PROF_VRENDER);
	static int lastvbl = 0;
	pvr_stats_t stats;
	pvr_get_stats(&stats);
	
	int i;
	
	static long long f2f_last_frame_start_us = 0;
	static long long f2f_last_frame_p2p_start_us = 0;
	
	long long f2f_last_frame_duration_us = timer_us_gettime64() - f2f_last_frame_start_us;
	static float avrg_lfd = 10;
	if (inkey == 'f') {
		printf("-------------avrg_lfd: %.2f ms\n", avrg_lfd);
		fflush(stdout);
	}
	avrg_lfd = avrg_lfd * 0.985 + f2f_last_frame_duration_us/1000.0f*0.015;
	
	prof_start(PROF_PVRWAIT);
	pvr_wait_ready();
	prof_end(PROF_PVRWAIT);
	f2f_last_frame_start_us = timer_us_gettime64();
	pvr_get_stats(&stats);
	
	//Calculate frame rate
	float CurFPS = 0;
	int vbls = stats.vbl_count - lastvbl;
	{
		
		int i, totvbls = 0;
		for(i = 0; i < VBLCNT; i++) {
			totvbls += FrameVBLs[i];
		}
		CurFPS = 60 / ((float)totvbls / VBLCNT);
		
		FrameVBLs[CurFrameVBL++] = vbls;
		//~ printf("%i %f   ", (int)(60 / stats.frame_rate), stats.frame_rate);
		if (CurFrameVBL >= VBLCNT)
			CurFrameVBL = 0;
	}
	lastvbl = stats.vbl_count;
	prof_start(PROF_TEX);
	{
			CurPVRBuff ^= 1;
			CurPVRTilesCxt = &PVRTilesCxtBnk[CurPVRBuff][0];
			CurPVRTilesCxtAlt = &PVRTilesCxtBnkAlt[CurPVRBuff][0];
			CurPVRTilesSprCxt = &PVRTilesSprCxtBnk[CurPVRBuff][0];
			CurPVRTilesSprCxtAlt = &PVRTilesSprCxtBnkAlt[CurPVRBuff][0];
		
		uintptr_t texstartint = (uintptr_t)PVRTilesTexture[CurPVRBuff];
		texstartint = 0x11000000 | (texstartint & 0xffffff);
		PVRVRAMData *texstart = (PVRVRAMData*)texstartint;
		
		struct pvr_dma_mode_reg_layout {
			int mode0, mode1;
		};
		#define PVR_DMA_FAST_MODE (*(volatile struct pvr_dma_mode_reg_layout*)0xA05F6884)
		PVR_DMA_FAST_MODE.mode0 = PVR_DMA_FAST_MODE.mode1 = 0;
		
		sqSetTargetAddress(texstart);
		
		short *cram = (short *)Raster0_CRam;
		if (Raster_ColorSplit) {
			short *cram2 = (short *)Raster1_CRam;
			PalToCodebook4(cram2,    sqGetWritePointer(texstart->CB[1].VDPPalette[0]));
			PalToCodebook4(cram2+16, sqGetWritePointer(texstart->CB[1].VDPPalette[1]));
			PalToCodebook4(cram2+32, sqGetWritePointer(texstart->CB[1].VDPPalette[2]));
			PalToCodebook4(cram2+48, sqGetWritePointer(texstart->CB[1].VDPPalette[3]));
		}
		PalToCodebook4(cram,    sqGetWritePointer(texstart->CB[0].VDPPalette[0]));
		PalToCodebook4(cram+16, sqGetWritePointer(texstart->CB[0].VDPPalette[1]));
		PalToCodebook4(cram+32, sqGetWritePointer(texstart->CB[0].VDPPalette[2]));
		PalToCodebook4(cram+48, sqGetWritePointer(texstart->CB[0].VDPPalette[3]));
		if (VDP_Reg.Set4 & 0x6) {
			//~ VRAM8x16ToTexture(VRam + ((VDP_Status & 0x10) ? 4 : 0), sqPrepare(texstart->VRAM));
			VRAM8x16ToTexture(VRam, sqGetWritePointer(texstart->VRAM));
		} else {
			VRAM8x8ToTexture2(VRam, sqGetWritePointer(texstart->VRAM));
		}
		
		extern void MakeBGTex(short *src, int count, void *dst);
		texstartint = (uintptr_t)PVRBGColorTexture[CurPVRBuff];
		texstartint = 0x11000000 | (texstartint & 0xffffff);
		MakeBGTex(Raster_BgColor, 224, sqGetWritePointer((void*)texstartint));
		//~ MakeBGTex(Raster_BgColor, 224, sqPrepare(PVRBGColorTexture[CurPVRBuff]));
	}
	prof_end(PROF_TEX);
	extern int H_Pix;
	
	//Render screen
	pvr_scene_begin();
		void *pc = sqPrepare((void*)PVR_TA_INPUT);
		
		pvr_list_begin(PVR_LIST_TR_POLY);
		LockList(PC_BLEND_POLY);
	//~ }
		if (Paused) {
			void *pc = sqPrepare((void*)PVR_TA_INPUT);
			
			{ //Force transparent list for normal KOS PVR driver
			pvr_context spr;
			pc_set_default_sprite(&spr);
			pc_set_list(&spr, PC_BLEND_POLY);
			sqCopy32(pc, &spr);
			}
			
			//Process and draw in-game pause menu
			extern void RunGameMenu();
			RunGameMenu();
		}
		
		if (Show_FPS)
			BMFprintf(emufont, 295,0,400, "%.1f", CurFPS);
		
		
		//~ if (Enable_Sprites)
			//~ NDrawSprites();
		
		pvr_list_finish();
		
	//*	
		//Cover up any VDP rendering with the bg color if the screen is off
		pvr_list_begin(PVR_LIST_OP_POLY);
			LockList(PC_OPAQUE_POLY);
			
			sqCopy32(pc, PVRBgColorCxt + CurPVRBuff);
			volatile pvr_vertex64s *v = pc;
			float bgdepth = (Raster_Lines[112] & RASTER_LINE_SCREEN_OFF) ? DEPTH_LAYER_S_HI + 0.5 : DEPTH_LAYER_BG;
			pv_set_cmd_vertex_eos(v);
			pv_set_submit_rect_cw(v, 0, SCREEN_START_Y, 1000, SCREEN_START_Y+224, 
				bgdepth);
			pv_set_uv16_0(v, pv_pack_uv16_static(0, 0));
			pv_set_uv16_1(v, pv_pack_uv16_static(0, 0));
			pv_set_uv16_2(v, pv_pack_uv16_static(224.0/256.0, 0));
			sqSubmit64B(v);
				
		pvr_list_finish();
	//*/
		//*
		pvr_list_begin(PVR_LIST_OP_MOD);
			extern void DrawMapSH();
			LockList(PC_OPAQUE_MOD);
			DrawMapSH();
		pvr_list_finish();
	//*/
		//~ pvr_list_begin(PVRVDP_TARGET_LIST == PC_PUNCHTHROUGH ? PVR_LIST_PT_POLY : PVR_LIST_TR_POLY);
		pvr_list_begin(PVR_LIST_PT_POLY);
		LockList(PC_PUNCHTHROUGH);
		//*
#ifdef VDP_HIRES
		screen_start_x = H_Pix == 320 ? 0 : (640-512)/2;
		//~ draw_rect_wh(screen_start_x, SCREEN_START_Y, H_Pix*2, 224*2, DEPTH_LAYER_BG, 0xff000000 | (red<<16) | (gre<<8) | blu, pc);
#else
#if VDP_STRETCH_256
		screen_start_x = 0;
		if (H_Pix == 320) {
			TW = 16;
			TSW = 2.0;
		} else {
			TW = 20;
			TSW = 2.5;
		}
#else
			screen_start_x = H_Pix == 320 ? 0 : (320-256)/2;
		//~ draw_rect_wh(screen_start_x, SCREEN_START_Y, H_Pix, 224, DEPTH_LAYER_BG, 0xff000000 | (red<<16) | (gre<<8) | blu, pc);
#endif
#endif

		if (Show_Rasters) {
			//Show raster lines
			for(i = 0; i < 224; i++) {
				if (Raster_Lines[i]) {
					int color = 0xff202020;
					color |= (Raster_Lines[i] & RASTER_LINE_VSCROLLA_CHANGE) ? 0x0000ff : 0;
					color |= (Raster_Lines[i] & RASTER_LINE_VSCROLLB_CHANGE) ? 0x00ff00 : 0;
					color |= (Raster_Lines[i] & RASTER_LINE_COLOR_CHANGE) ? 0xff0000 : 0;
						draw_rect_wh(screen_start_x, SCREEN_START_Y+i, 8, 1, 15, color, pc);
					color = 0xff202020;
					color |= (Raster_Lines[i] & RASTER_LINE_MAPA_CHANGE) ? 0x0000ff : 0;
					color |= (Raster_Lines[i] & RASTER_LINE_WIN_CHANGE) ? 0x0000ff : 0;
					color |= (Raster_Lines[i] & RASTER_LINE_MAPB_CHANGE) ? 0x00ff00 : 0;
					//~ color |= (Raster_Lines[i] & RASTER_LINE_SPRLIST_CHANGE) ? 0xff0000 : 0;
						draw_rect_wh(screen_start_x+8, SCREEN_START_Y+i, 8, 1, 15, color, pc);
					if (Raster_Lines[i] & RASTER_LINE_SPRLIST_CHANGE)
						draw_rect_wh(screen_start_x+16, SCREEN_START_Y+i, 8, 1, 15, 0xffff0000, pc);
					//~ if (!(Raster_Lines[i] & RASTER_LINE_SCREEN_OFF))
				}
			}
		}

//Screen border
		//#define SCREEN_BORDER_COLOR	0xc0300000
		#define SCREEN_BORDER_COLOR	0xff000000
		#define SCREEN_BORDER_WIDTH	64
#if !defined(VDP_HIRES)
		draw_rect_wh(screen_start_x - SCREEN_BORDER_WIDTH, SCREEN_START_Y-SCREEN_BORDER_WIDTH, SCREEN_BORDER_WIDTH, 224+SCREEN_BORDER_WIDTH, 15, SCREEN_BORDER_COLOR, pc);	//Left edge
		draw_rect_wh(screen_start_x, SCREEN_START_Y-SCREEN_BORDER_WIDTH, H_Pix+SCREEN_BORDER_WIDTH, SCREEN_BORDER_WIDTH, 15, SCREEN_BORDER_COLOR, pc);	//Top edge
		if (!VDP_STRETCH_256) {
			draw_rect_wh(H_Pix + screen_start_x, SCREEN_START_Y, SCREEN_BORDER_WIDTH, 224, 15, SCREEN_BORDER_COLOR, pc);	//Right edge
			draw_rect_wh(screen_start_x - SCREEN_BORDER_WIDTH, SCREEN_START_Y+224, H_Pix+SCREEN_BORDER_WIDTH*2, SCREEN_BORDER_WIDTH, 15, SCREEN_BORDER_COLOR, pc);	//Bottom edge
		}
#endif

		//*/
		//Draw screen
		prof_start(PROF_MAPA);
		if (Enable_MapA == 1) {
			NDrawMapA();
		} else if (Enable_MapA == 2) {
			DrawMapPVR(0, DEPTH_LAYER_A_LO, DEPTH_LAYER_A_HI);
			if (Enable_Win)
				DrawMapWindow(DEPTH_LAYER_A_LO+0.1, DEPTH_LAYER_A_HI+0.1);
		}
		prof_end(PROF_MAPA);
		
		prof_start(PROF_MAPB);
		if (Enable_MapB == 1) {
			NDrawMapB();
		} else if (Enable_MapB == 2) {
			DrawMapPVR(1, DEPTH_LAYER_B_LO, DEPTH_LAYER_B_HI);
		}
		prof_end(PROF_MAPB);
		
		prof_start(PROF_SPR);
		//~ if (!BlendSprites) {
			if (Enable_Sprites) {
				if (Raster_ColorSplit)
					NDrawSpritesR();
				else
					NDrawSpritesNR();
			}
		//~ }
		prof_end(PROF_SPR);
				
			
		////////}
		
		
		//~ int i;

	
		//if (Raster_ColorSplit)
		//	draw_rect_wh(screen_start_x, SCREEN_START_Y + Raster_ColorSplitLine-1, 3, 3, 20, 0xff00ffff, pc);
		
		//Profile gauge
	#if 0
		#define PROF_TICK_VPOS	(230.0f)
		#define PROF_TICK_PIXELS	(150.0f)
	#else
	#if 0
		#define PROF_TICK_VPOS	(400.0f)
		#define PROF_TICK_PIXELS	(200.0f)
	#else
		#define PROF_TICK_VPOS	(225.0f)
		#define PROF_TICK_PIXELS	(200.0f)
	#endif
	#endif
	
	#define WHITE 0xffffffff
	#define GREEN 0xff00ff00
	#define DARK_GREEN 0xff008000
	#define RED 0xffa00000

		if (Show_Timing) {
			//float frametime_ms = last_frame_duration_us / 1000.0f;
			#define frame_length	(16.666f)
			#define us2ms(a) ((a) / 1000.0f)
			#define bar(time) ((time) / frame_length * PROF_TICK_PIXELS)
			#define bar_us(time) bar(us2ms(time))
			draw_rect_wh(bar(frame_busy_avrg),PROF_TICK_VPOS+2, 1, 8, 19.55, 0xffffffff, pc);	//average f2f time
			draw_rect_wh(bar(avrg_lfd),PROF_TICK_VPOS+2, 1, 8, 19.56, 0xffffff00, pc);	//average f2f time
			draw_rect_wh(0,PROF_TICK_VPOS+2, bar_us(f2f_last_frame_duration_us), 8, 19.5, GREEN, pc);	//f2f time
			//~ draw_rect_wh(0,PROF_TICK_VPOS+2+8, bar_us(f2f_last_frame_duration_p2p_us), 8, 19.5, 0xff008000, pc);	//f2f time
			//~ draw_rect_wh(0,PROF_TICK_VPOS+2+8, bar(stats.frame_last_time), 8, 19.5, 0xff008000, pc);	//f2f time
			draw_rect_wh(0,PROF_TICK_VPOS+2+8, bar(vbls * frame_length), 8, 19.5, 0xff008000, pc);	//f2f time
			draw_rect_wh(0,PROF_TICK_VPOS+2+8, bar((float)stats.frame_rate / 16.666f), 8, 19.6, RED, pc);	//render
		#ifdef PROFILER_GENS4ALL
			draw_rect_wh(0,PROF_TICK_VPOS+2, bar(last_frame_busy), 8, 19.5, GREEN, pc);	//f2f time
			draw_rect_wh(0,PROF_TICK_VPOS+2, bar_us(prof_last_frame_time[PROF_M68K]), 3, 19.7, 0xffff0000, pc);	//68k time
			draw_rect_wh(0,PROF_TICK_VPOS-2, bar_us(prof_last_frame_time[PROF_MAINLOOP])*10, 3, 19.7, 0xffa0a0a0, pc);	//mainloop
			draw_rect_wh(0,PROF_TICK_VPOS+5, bar_us(prof_last_frame_time[PROF_YMH]), 3, 19.7, 0xff4040ff, pc);	//fm synth
			draw_rect_wh(0,PROF_TICK_VPOS+8, bar_us(render_total_us), 3, 19.7, 0xffa040a0, pc);	//render
			draw_rect_wh(0,PROF_TICK_VPOS+11, bar_us(prof_last_frame_time[PROF_Z80]), 3, 19.7, 0xffb0b0b0, pc);	//z80
		#endif
			for(i = 0; i < 17; i++) {
				float h = (i % 5 != 0) ? 4 : 8;
				draw_rect_wh(i * (PROF_TICK_PIXELS/frame_length),PROF_TICK_VPOS, 1, h, 20, WHITE, pc);	//millisec
			}
			draw_rect_wh(PROF_TICK_PIXELS,PROF_TICK_VPOS, 1, 10, 20, WHITE, pc);	//1 frame
			draw_rect_wh(PROF_TICK_PIXELS*2,PROF_TICK_VPOS, 1, 10, 20, WHITE, pc);	//2 frame
			draw_rect_wh(PROF_TICK_PIXELS*3,PROF_TICK_VPOS, 1, 10, 20, WHITE, pc);	//3 frame
		}
	//~ if (PVRVDP_TARGET_LIST == PC_PUNCHTHROUGH) {
		pvr_list_finish();
		
	pvr_scene_finish();
	
prof_end(PROF_VRENDER);

}

#else


void Blit_Dreamcast(unsigned char *Dest, const int x, const int y, const int offset)
{
	int i;

	register U32 *s = (U32 *) MD_Screen + 4;
	register U32 *d = (U32 *)(void *) (0xe0000000 | (((unsigned long)Dest) & 0x03ffffe0));
	const U32 nx = (x >> 4);
	const U32 soff = (offset/4);
	const U32 doff = (640 - x*2)/4;

	QACR0 = ((((unsigned int)Dest)>>26)<<2)&0x1c;
	QACR1 = ((((unsigned int)Dest)>>26)<<2)&0x1c;

	for(i=0;i<y;i++)
	{
		register int n=nx;
		while(n--)
		{
			asm("pref @%0" : : "r" (s + 8));
			d[0] = *(s++);
			d[1] = *(s++);
			d[2] = *(s++);
			d[3] = *(s++);
			d[4] = *(s++);
			d[5] = *(s++);
			d[6] = *(s++);
			d[7] = *(s++);
			asm("pref @%0" : : "r" (d));
			d += 8;
		}
		s+=soff;
		d+=doff;
	}
	d = (unsigned int *)0xe0000000;
	d[0] = d[8] = 0;
}
#endif

#endif
