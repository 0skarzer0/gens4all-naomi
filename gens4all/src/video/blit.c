
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
void Blit_Dreamcast(unsigned char *Dest, const int x, const int y, const int offset)
{
	//PrintTimer();
//	return;
	extern long long last_frame_duration_us;
	
	static long long f2f_last_frame_start_us = 0;
	static long long f2f_last_frame_p2p_start_us = 0;
	static int fskipped = 0;
	
	long long f2f_last_frame_duration_us = timer_us_gettime64() - f2f_last_frame_start_us;
	//~ printf("%.2f ms\n", f2f_last_frame_duration_us / 1000.f);
	//~ f2f_last_frame_start_us = timer_us_gettime64();
	
//	extern int frameskipping;
//	if (frameskipping) {
//		return;
//	}
	
	//*
	#define frameskiptime (17.0*1000)
	if (0 && !fskipped && ((last_frame_duration_us > frameskiptime) || (f2f_last_frame_duration_us > frameskiptime))) {
		//printf("S"); fflush(stdout);
		fskipped = 1;
		return;
	}
	fskipped = 0;
	
	/*
	if (!fskipped && (vbls > 1)) {
		//~ printf("S"); fflush(stdout);
		fskipped = 1;
		return;
	}
	fskipped = 0;
	//*/
	
//	vid_waitvbl();
//	return;
	
	short *cram = (short *)CRam;
	int vdpbgcolor = cram[VDP_Reg.BG_Color];
	int red = vdpbgcolor & 0xe;
	int gre = (vdpbgcolor>>4) & 0xe;
	int blu = (vdpbgcolor>>8) & 0xe;
	red <<= 4;
	gre <<= 4;
	blu <<= 4;
//	red |= red >> 3;
//	gre |= gre >> 3;
//	blu |= blu >> 3;
//	red |= red >> 3;
//	gre |= gre >> 3;
//	blu |= blu >> 3;
//	vid_border_color(red, gre, blu);
	
	pvr_wait_ready();
	
	pvr_stats_t stats;
	pvr_get_stats(&stats);
	
	
	static int lastvbl = 0;
	int vbls = stats.vbl_count - lastvbl;
	lastvbl = stats.vbl_count;
	
	long long f2f_last_frame_duration_p2p_us = timer_us_gettime64() - f2f_last_frame_p2p_start_us;
	f2f_last_frame_p2p_start_us = timer_us_gettime64();
	f2f_last_frame_start_us = timer_us_gettime64();
	
prof_start(PROF_VRENDER);
	//Load VRAM texture
	if (0) {
		CurPVRBuff ^= 1;
		CurPVRTilesCxt = &PVRTilesCxtBnk[CurPVRBuff][0];
		CurPVRTilesSprCxt = &PVRTilesSprCxtBnk[CurPVRBuff][0];
		VRAMToTexture(VRam, PVRTilesTexture[CurPVRBuff]);
		//~ pvr_txr_load_dma(&VdpTiles, PVRTilesTexture[CurPVRBuff], sizeof(VdpTiles), 1, 0, 0);
	}
	if (1) {
		PalToCodebook(cram, (void*)VdpTiles.PaletteCodebook+2048*0);
		PalToCodebook(cram+16, (void*)VdpTiles.PaletteCodebook+2048*1);
		PalToCodebook(cram+32, (void*)VdpTiles.PaletteCodebook+2048*2);
		PalToCodebook(cram+48, (void*)VdpTiles.PaletteCodebook+2048*3);
		VRAMToTexture(VRam, VdpTiles.VRAM);
		purge_cache();
		
		//TODO bank swap
#ifdef PVRVDP_DBL_BUFF
		CurPVRBuff ^= 1;
		CurPVRTilesCxt = &PVRTilesCxtBnk[CurPVRBuff][0];
		CurPVRTilesSprCxt = &PVRTilesSprCxtBnk[CurPVRBuff][0];
		pvr_txr_load_dma(&VdpTiles, PVRTilesTexture[CurPVRBuff], sizeof(VdpTiles), 1, 0, 0);
#else
		pvr_txr_load_dma(&VdpTiles, PVRTilesTexture, sizeof(VdpTiles), 1, 0, 0);
#endif
		//pvr_txr_load_dma(&VdpTiles, PVRTilesTexture, sizeof(VdpTiles), 1, 0, 0);
	}
	
	//Render screen
	pvr_scene_begin();
		pvr_list_begin(PVRVDP_TARGET_LIST == PC_PUNCHTHROUGH ? PVR_LIST_PT_POLY : PVR_LIST_TR_POLY);
		
		short *sprlist = (short*)(((VDP_Reg.Spr_Att_Adr & 0x7F) << 9) + VRam);
		
		void *pc = sqPrepare((void*)PVR_TA_INPUT);
		
/*		draw_rect_wh(0,0, 10,30,  0.0, 0x80808080, pc);
		draw_rect_wh(10,0, 10,30,  0.1, 0x80ffffff, pc);
		draw_rect_wh(20,0, 10,30,  0.2, 0x80ffff00, pc);
		draw_rect_wh(30,0, 10,30,  0.3, 0x80ff00ff, pc);
		draw_rect_wh(40,0, 10,30,  0.4, 0x8000ffff, pc);
		draw_rect_wh(50,0, 10,30,  0.5, 0x80ff0000, pc);
		draw_rect_wh(60,0, 10,30,  0.6, 0x800000ff, pc);
		draw_rect_wh(70,0, 10,30,  0.7, 0x8000ff00, pc);
		draw_rect_wh(80,0, 10,30,  0.8, 0x80000000, pc); */
		
//		DrawVRAMPVRManual(0, 224, 15);
		
		//DrawLayerCellPVR();
		//DrawMapsPVR();
		
		//extern unsigned short MD_Palette[256];
		extern struct Reg_VDP_Type VDP_Reg;
		extern int H_Pix;
		
		//Draw Bg color
		//int bgcolor = MD_Palette[0];
		draw_rect_wh(SCREEN_START_X, SCREEN_START_Y, H_Pix, 224, DEPTH_LAYER_BG, 0xff000000 | (red<<16) | (gre<<8) | blu, pc);
		
		//Is VDP enabled?
		extern int Force_Screen_On_Hack;
		if (Force_Screen_On_Hack || (VDP_Reg.Set2 & 0x40)) {
			//Draw screen
			DrawMapWindow(DEPTH_LAYER_A_LO+0.1, DEPTH_LAYER_A_HI+0.1);
			DrawMapPVR(0, DEPTH_LAYER_A_LO, DEPTH_LAYER_A_HI);
			DrawMapPVR(1, DEPTH_LAYER_B_LO, DEPTH_LAYER_B_HI);
			DrawSprites();
		}
		
		//Screen border
		//#define SCREEN_BORDER_COLOR	0xc0300000
		#define SCREEN_BORDER_COLOR	0xff000000
		#define SCREEN_BORDER_WIDTH	16
		draw_rect_wh(SCREEN_START_X - SCREEN_BORDER_WIDTH, SCREEN_START_Y-SCREEN_BORDER_WIDTH, SCREEN_BORDER_WIDTH, 224+16, 15, SCREEN_BORDER_COLOR, pc);	//Left edge
		draw_rect_wh(H_Pix + SCREEN_START_X, SCREEN_START_Y, SCREEN_BORDER_WIDTH, 224, 15, SCREEN_BORDER_COLOR, pc);	//Right edge
		draw_rect_wh(SCREEN_START_X, SCREEN_START_Y-SCREEN_BORDER_WIDTH, H_Pix+SCREEN_BORDER_WIDTH, SCREEN_BORDER_WIDTH, 15, SCREEN_BORDER_COLOR, pc);	//Top edge
		draw_rect_wh(SCREEN_START_X, SCREEN_START_Y+224, H_Pix+SCREEN_BORDER_WIDTH, SCREEN_BORDER_WIDTH, 15, SCREEN_BORDER_COLOR, pc);	//Bottom edge
		
prof_end(PROF_VRENDER);
		
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
		#define PROF_TICK_PIXELS	(100.0f)
	#endif
	#endif
	
		
		//float frametime_ms = last_frame_duration_us / 1000.0f;
		#define frame_length	(16.666f)
		#define us2ms(a) ((a) / 1000.0f)
		#define bar(time) ((time) / frame_length * PROF_TICK_PIXELS)
		#define bar_us(time) bar(us2ms(time))
		draw_rect_wh(0,PROF_TICK_VPOS+2, bar_us(f2f_last_frame_duration_us), 8, 19.5, 0xff00ff00, pc);	//f2f time
		//~ draw_rect_wh(0,PROF_TICK_VPOS+2+8, bar_us(f2f_last_frame_duration_p2p_us), 8, 19.5, 0xff008000, pc);	//f2f time
		//~ draw_rect_wh(0,PROF_TICK_VPOS+2+8, bar(stats.frame_last_time), 8, 19.5, 0xff008000, pc);	//f2f time
		draw_rect_wh(0,PROF_TICK_VPOS+2+8, bar(vbls * frame_length), 8, 19.5, 0xff008000, pc);	//f2f time
		draw_rect_wh(0,PROF_TICK_VPOS+2+8, bar((float)stats.rnd_last_time), 8, 19.7, 0xffa00000, pc);	//f2f time
		draw_rect_wh(0,PROF_TICK_VPOS+2, bar_us(prof_last_frame_time[PROF_M68K]), 3, 19.7, 0xffff0000, pc);	//68k time
		draw_rect_wh(0,PROF_TICK_VPOS+5, bar_us(prof_last_frame_time[PROF_YMH]), 3, 19.7, 0xff4040ff, pc);	//fm synth
		draw_rect_wh(0,PROF_TICK_VPOS+8, bar_us(prof_last_frame_time[PROF_VRENDER]), 3, 19.7, 0xffa040a0, pc);	//render
		draw_rect_wh(0,PROF_TICK_VPOS+11, bar_us(prof_last_frame_time[PROF_Z80]), 3, 19.7, 0xffb0b0b0, pc);	//z80
		
		int i;
		for(i = 0; i < 17; i++) {
			float h = (i % 5 != 0) ? 4 : 8;
			draw_rect_wh(i * (PROF_TICK_PIXELS/frame_length),PROF_TICK_VPOS, 1, h, 20, 0xffffffff, pc);	//millisec
		}
		draw_rect_wh(PROF_TICK_PIXELS,PROF_TICK_VPOS, 1, 10, 20, 0xffffffff, pc);	//1 frame
		draw_rect_wh(PROF_TICK_PIXELS*2,PROF_TICK_VPOS, 1, 10, 20, 0xffffffff, pc);	//2 frame
		draw_rect_wh(PROF_TICK_PIXELS*3,PROF_TICK_VPOS, 1, 10, 20, 0xffffffff, pc);	//3 frame
		
		
		pvr_list_finish();
	pvr_scene_finish();
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
