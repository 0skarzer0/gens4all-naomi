#include <stdio.h>
#include <math.h>

#include "gens4all.h"
#include "vdp_io.h"
#include "vdp_rend.h"
#include "blit.h"
#include "scrshot.h"
//#include "net.h"
#include "misc.h"
//#include "cdda_mp3.h"
#include "input.h"

#ifdef DREAMCAST
//#include <SDL_dreamcast.h>
#endif

int DisableInterlace;

#ifndef DEBUG_VDP
SDL_Surface *surface = 0;
#endif

Uint32 Last_Time = 0;
Uint32 New_Time = 0;
Uint32 Used_Time = 0;

int x_offset, y_offset = 0;

int Flag_Clr_Scr = 0;
int Sleep_Time;
int FS_VSync;
int W_VSync;
int Stretch;
int Blit_Soft;
int Effect_Color = 7;
int FPS_Style = EMU_MODE | BLANC;
int Message_Style = EMU_MODE | BLANC | SIZE_X2;
int Kaillera_Error = 0;

void (*Blit_FS) (unsigned char *Dest, int pitch, int x, int y, int offset);
void (*Blit_W) (unsigned char *Dest, int pitch, int x, int y, int offset);
int (*Update_Frame) ();
int (*Update_Frame_Fast) ();

static char Info_String[1024] = "";
static int Message_Showed = 0;
static unsigned int Info_Time = 0;


static void win2linux (char *str)
{
  char *tmp = str;
  for (; *tmp; ++tmp)
    {
      switch ((unsigned char)*tmp)
	{
	case 0xE7:
	  *tmp = 'c';
	  break;		//ç
	case 0xE8:
	  *tmp = 'e';
	  break;		//è
	case 0xE9:
	  *tmp = 'e';
	  break;		//é
	case 0xEA:
	  *tmp = 'e';
	  break;		//ê
	case 0xE0:
	  *tmp = 'a';
	  break;		//à
	case 0xEE:
	  *tmp = 'i';
	  break;		//î
	default:
	  break;
	}
    }
}

void Put_Info (char *Message, int Duree)
{
  if (Show_Message)
    {
      strcpy (Info_String, Message);
      win2linux (Info_String);
      Info_Time = GetTickCount () + Duree;
      Message_Showed = 1;
    }
}


int Init_Fail (int hwnd, char *err)
{
	Shutdown_Video_SubSystem();
	puts (err);
	//MessageBox(hwnd, err, "Oups ...", MB_OK);
	//DestroyWindow(hwnd);
	exit (0);
	return 0;
}

#include "pvrvdp/pvr_rend.h"

int Init_Video_SubSystem(int w, int h, int flags)
{
#ifndef DEBUG_VDP
#ifdef DREAMCAST
	//SDL_DC_SetVideoDriver(SDL_DC_DIRECT_VIDEO);
#endif
#if 0
      SDL_InitSubSystem(SDL_INIT_VIDEO);

  if ((surface = SDL_SetVideoMode (w, h, 16, flags)) == 0)
    {
      fprintf (stderr, "Error creating SDL primary surface : %s\n",
	       SDL_GetError ());
      exit (0);
    }
#endif
/*
  if (flags & SDL_FULLSCREEN)
    {
      SDL_ShowCursor (SDL_DISABLE);
    }
*/
#endif

	
	if (VDP_STRETCH_256) {
		vid_mode_t vm640x240 = {
			DM_640x480,
			640, 240,
			VID_LINEDOUBLE,
			
			CT_VGA,
			1,
			
			0x20C/2, 0x359,
			0xAC, 0x28,
			
			0x15, 0x104,
			
			0x7E, 0x345,
			0x24, 0x204,
			0, 1,
			{ 0, 0, 0, 0 }
		};
		vid_set_mode_ex(&vm640x240);
	} else if (1) {
		#ifndef VDP_HIRES
			vid_init(DM_320x240, PM_RGB565);
		#endif
		
		if (DisableInterlace) {
			volatile int *sync = (int*)0xa05f80d0;
			*sync = *sync & ~0x10;
			PVR_SET(PVR_SCALER_CFG, 0x400);
		}
		
	} else  {
		vid_init(DM_320x240, PM_RGB888);
	}
//	vid_border_color(255, 0, 0);
//	sleep(5);
//*
	pvr_init_params_t pvr_params = {
		{ PVR_BINSIZE_8, PVR_BINSIZE_16, PVR_BINSIZE_32, PVR_BINSIZE_16, PVR_BINSIZE_32 },
		(int)1.5*1024*1024,
		0,
		0,
		0, //sort
	};
	if (pvr_init(&pvr_params)) {
		printf("Init error\n");
		fflush(stdout);
	}
	pvr_set_bg_color(0,0,0.21);

	PVR_GET(PVR_FB_CFG_2) &= ~0x8;	//disable dither
	
	//Speed up plunchthrough
	//https://dcemulation.org/phpBB/viewtopic.php?t=102994&start=20#p1043868
//	PVR_GET(PVR_UNK_0098) = 0x00801008;
	//~ PVR_GET(0x011c) = 0x40;
	
	//~ PVR_GET(PVR_UNK_007c) = 0x0027df77;
	//~ PVR_GET(PVR_UNK_007C) = 0x0027dfff;
	//~ int v = 0x00200077;
	//~ v |= 8 << 14;
	//~ v |= 8 << 8;
	//~ PVR_GET(PVR_UNK_007C) = v;
	pvr_set_shadow_scale(1, 0.5);
	
	InitPVRTexture();
	InitTileUVTable();
//*/	

  return 1;

}

void Shutdown_Video_SubSystem(void)
{
// CHUI: Are you crazy!?
	SDL_QuitSubSystem (SDL_INIT_VIDEO);
}
#if 1
int Flip (void)
{
	Blit_Dreamcast (0,0,0,0);
	return 1;
}

#else
int Flip (void)
{
#ifdef DEBUG_VDP
	dbgsum("MD_Screen(Flip)",MD_Screen,336 * 240 *2);
#else
  int Dep, i;
  static float FPS = 0.0f, frames[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  static unsigned int old_time = 0, view_fps = 0, index_fps = 0, freq_cpu[2] =
    { 0, 0 };
  unsigned int new_time[2];


  prof_start(PROF_BLIT);
  if (Fast_Blur)
    Half_Blur ();

  if ((VDP_Reg.Set4 & 0x1) || (Debug))
    {
      Dep = 0;
    }
  else
    {
      Dep = 64;
    }


	if (Flag_Clr_Scr == 1)
	{
		//SDL_FillRect (surface, NULL, 0);
		Flag_Clr_Scr = 0;
	}

#ifndef DREAMCAST
	SDL_LockSurface (surface);
	if (Full_Screen)
	{
		Blit_FS ((unsigned char *) surface->pixels +
		   (surface->w * (240 - VDP_Num_Vis_Lines)) + Dep,
		   surface->w * 2, 320 - Dep, VDP_Num_Vis_Lines, 32 + Dep * 2);
	}
	else
	{
		Blit_W ((unsigned char *) surface->pixels +
			(surface->w * (240 - VDP_Num_Vis_Lines)) + Dep,
			surface->w * 2, 320 - Dep, VDP_Num_Vis_Lines, 32 + Dep * 2);
	}
	SDL_UnlockSurface (surface);
#else
	Blit_Dreamcast ((unsigned char *) surface->pixels +
		   (surface->w * (240 - VDP_Num_Vis_Lines)) + Dep,
		   320 - Dep, VDP_Num_Vis_Lines, 32 + Dep * 2);
#endif


#if !defined(DEBUG_GENS4ALL) && !defined(PROFILER_GENS4ALL)
  if (Message_Showed)
    {
      if (GetTickCount () > Info_Time)
	{
	  Message_Showed = 0;
	  strcpy (Info_String, "");
	}
      else
	Print_Text (surface, Info_String, 10, 215, Message_Style);

    }
  else if (Show_FPS)
    {
      if (freq_cpu[0] > 1)	// accurate timer ok
	{
	  if (++view_fps >= 16)
	    {
//	      GetPerformanceCounter ((long long *) new_time);
	      if (new_time[0] != old_time)
		{
		  FPS =
		    (float) (freq_cpu[0]) * 16.0f / (float) (new_time[0] -
							     old_time);
		  sprintf (Info_String, "%.1f", FPS);
		}
	      else
		{
		  sprintf (Info_String, "too much...");
		}

	      old_time = new_time[0];
	      view_fps = 0;
	    }
	}
      else if (freq_cpu[0] == 1)	// accurate timer not supported
	{
	  if (++view_fps >= 10)
	    {
	      new_time[0] = GetTickCount ();

	      if (new_time[0] != old_time)
		frames[index_fps] = 10000 / (float) (new_time[0] - old_time);
	      else
		frames[index_fps] = 2000;

	      index_fps++;
	      index_fps &= 7;
	      FPS = 0.0f;

	      for (i = 0; i < 8; i++)
		FPS += frames[i];

	      FPS /= 8.0f;
	      old_time = new_time[0];
	      view_fps = 0;
	    }

	  sprintf (Info_String, "%.1f", FPS);
	}
      else
	{
	  //GetPerformanceFrequency ((long long *) freq_cpu);
	  if (freq_cpu[0] == 0)
	    freq_cpu[0] = 1;

	  sprintf (Info_String, "", FPS);
	}

      Print_Text (surface, Info_String, 10, 215, FPS_Style);
    }
#endif

//	SDL_Flip (surface);

#endif
  prof_end(PROF_BLIT);
  return 1;
}
#endif

int Update_Gens_Logo (void)
{
#if 0
#ifndef DEBUG_VDP
  int i, j, m, n;
  static short tab[64000], Init = 0;
  static float renv = 0, /*ang = 0, */ zoom_x = 0, zoom_y = 0, pas;
  unsigned short c;

	if (!Init)
	{
		SDL_Surface *Logo;
		
		Logo = SDL_LoadBMP (DATA_PREFIX "/gens_big.bmp");
		
		SDL_LockSurface (Logo);
		memcpy (tab, Logo->pixels, 47000 /*64000 */ );
		SDL_UnlockSurface (Logo);
		
		pas = 0.05;
		Init = 1;
	}

	renv += pas;
	zoom_x = sin (renv);
	if (zoom_x == 0.0)
	zoom_x = 0.0000001;
	zoom_x = (1 / zoom_x) * 1;
	zoom_y = 1;
	
	if (VDP_Reg.Set4 & 0x1)
	{
		for (j = 0; j < 240; j++)
		{
			for (i = 0; i < 320; i++)
			{
				m = (float) (i - 160) * zoom_x;
				n = (float) (j - 120) * zoom_y;

				if ((m < 130) && (m >= -130) && (n < 90) && (n >= -90))
				{
					c = tab[m + 130 + (n + 90) * 260];
					if ((c > 31) || (c < 5))
						MD_Screen[j*336 + i + 8] = c;
				}
			}
		}
	}
	else
	{
		for (j = 0; j < 240; j++)
		{
			for (i = 0; i < 256; i++)
			{
				m = (float) (i - 128) * zoom_x;
				n = (float) (j - 120) * zoom_y;

				if ((m < 130) && (m >= -130) && (n < 90) && (n >= -90))
				{
					c = tab[m + 130 + (n + 90) * 260];
					if ((c > 31) || (c < 5))
						MD_Screen[j*336 + i + 8] = c;
				}
		    }
		}
	}

	Half_Blur ();
	Flip ();

#endif
#endif
	return 1;
}


int Update_Crazy_Effect (void)
{
#if 0
  int i, j, offset;
  int r = 0, v = 0, b = 0, prev_l, prev_p;
  int RB, G;

  for (offset = 336 * 240, j = 0; j < 240; j++)
    {
      for (i = 0; i < 336; i++, offset--)
	{
	  prev_l = MD_Screen[offset - 336];
	  prev_p = MD_Screen[offset - 1];

	  if (Mode_555 & 1)
	    {
	      RB = ((prev_l & 0x7C1F) + (prev_p & 0x7C1F)) >> 1;
	      G = ((prev_l & 0x03E0) + (prev_p & 0x03E0)) >> 1;

	      if (Effect_Color & 0x4)
		{
		  r = RB & 0x7C00;
		  if (rand () > 0x2C00)
		    r += 0x0400;
		  else
		    r -= 0x0400;
		  if (r > 0x7C00)
		    r = 0x7C00;
		  else if (r < 0x0400)
		    r = 0;
		}

	      if (Effect_Color & 0x2)
		{
		  v = G & 0x03E0;
		  if (rand () > 0x2C00)
		    v += 0x0020;
		  else
		    v -= 0x0020;
		  if (v > 0x03E0)
		    v = 0x03E0;
		  else if (v < 0x0020)
		    v = 0;
		}

	      if (Effect_Color & 0x1)
		{
		  b = RB & 0x001F;
		  if (rand () > 0x2C00)
		    b++;
		  else
		    b--;
		  if (b > 0x1F)
		    b = 0x1F;
		  else if (b < 0)
		    b = 0;
		}
	    }
	  else
	    {
	      RB = ((prev_l & 0xF81F) + (prev_p & 0xF81F)) >> 1;
	      G = ((prev_l & 0x07C0) + (prev_p & 0x07C0)) >> 1;

	      if (Effect_Color & 0x4)
		{
		  r = RB & 0xF800;
		  if (rand () > 0x2C00)
		    r += 0x0800;
		  else
		    r -= 0x0800;
		  if (r > 0xF800)
		    r = 0xF800;
		  else if (r < 0x0800)
		    r = 0;
		}

	      if (Effect_Color & 0x2)
		{
		  v = G & 0x07C0;
		  if (rand () > 0x2C00)
		    v += 0x0040;
		  else
		    v -= 0x0040;
		  if (v > 0x07C0)
		    v = 0x07C0;
		  else if (v < 0x0040)
		    v = 0;
		}

	      if (Effect_Color & 0x1)
		{
		  b = RB & 0x001F;
		  if (rand () > 0x2C00)
		    b++;
		  else
		    b--;
		  if (b > 0x1F)
		    b = 0x1F;
		  else if (b < 0)
		    b = 0;
		}
	    }

	  MD_Screen[offset] = r + v + b;
	}
    }

  Flip ();
#endif
  return 1;
}


int Update_Emulation (void)
{
  static int Over_Time = 0;
  int current_div;

  if (Frame_Skip != -1)
    {
#ifndef NO_SOUND
      if (Sound_Enable) {
	      Write_Sound_Buffer (NULL);
	      
	     
	}
#endif

      Update_Controllers ();

      if (Frame_Number++ < Frame_Skip)
	{
	  Update_Frame_Fast ();
	}
      else
	{
	  Frame_Number = 0;
	  Update_Frame ();
	  Flip ();
	}
    }
  else
    {
      if (1 && Sound_Enable)
	{
		Write_Sound_Buffer (NULL);
		 if (0 && !Use_SDL_Audio) {
			//~ printf("B"); fflush(stdout);
			//~ snd_stream_start(game_stream, Sound_Rate, Sound_Stereo);
			//~ int ret = snd_stream_poll(game_stream);
			//~ if (ret) {
				//~ printf("SSP: %i\n", ret);fflush(stdout);
			//~ }
		}
		Update_Controllers ();
		Update_Frame ();
		if (Render_Point == 1)
			Flip ();	//DC flip with sound
	}
      else
	{
	  if (CPU_Mode)
	    current_div = 20;
	  else
	    current_div = 16 + (Over_Time ^= 1);

	  New_Time = GetTickCount ();
	  Used_Time += (New_Time - Last_Time);
	  Frame_Number = Used_Time / current_div;
	  Used_Time %= current_div;
	  Last_Time = New_Time;

	  if (Frame_Number > 8)
	    Frame_Number = 8;

	  for (; Frame_Number > 1; Frame_Number--)
	    {
	      Update_Controllers ();
	      Update_Frame_Fast ();
	    }

	  if (Frame_Number)
	    {
	      Update_Controllers ();
	      Update_Frame ();
	      Flip ();
	    }
	  else
	    Sleep (Sleep_Time);
	}
    }

  return 1;
}


int Update_Emulation_One (void)
{
  Update_Controllers ();
  Update_Frame ();
  Flip ();

  return 1;
}


int Update_Emulation_Netplay (int player, int num_player)
{
#if 0
  static int Over_Time = 0;
  int current_div;

  if (CPU_Mode)
    current_div = 20;
  else
    current_div = 16 + (Over_Time ^= 1);

  New_Time = GetTickCount ();
  Used_Time += (New_Time - Last_Time);
  Frame_Number = Used_Time / current_div;
  Used_Time %= current_div;
  Last_Time = New_Time;

  if (Frame_Number > 6)
    Frame_Number = 6;

  for (; Frame_Number > 1; Frame_Number--)
    {
      if (Sound_Enable)
	{
	  if (WP == Get_Current_Seg ())
	    WP = (WP - 1) & (Sound_Segs - 1);
	  Write_Sound_Buffer (NULL);
	  WP = (WP + 1) & (Sound_Segs - 1);
	}

      Scan_Player_Net (player);
      if (Kaillera_Error != -1)
	Kaillera_Error =
	  Kaillera_Modify_Play_Values ((void *) (Kaillera_Keys), 2);
      //Kaillera_Error = Kaillera_Modify_Play_Values((void *) (Kaillera_Keys), 2);
      Update_Controllers_Net (num_player);
      Update_Frame_Fast ();
    }

  if (Frame_Number)
    {
      if (Sound_Enable)
	{
	  if (WP == Get_Current_Seg ())
	    WP = (WP - 1) & (Sound_Segs - 1);
	  Write_Sound_Buffer (NULL);
	  WP = (WP + 1) & (Sound_Segs - 1);
	}

      Scan_Player_Net (player);
      if (Kaillera_Error != -1)
	Kaillera_Error =
	  Kaillera_Modify_Play_Values ((void *) (Kaillera_Keys), 2);
      //Kaillera_Error = Kaillera_Modify_Play_Values((void *) (Kaillera_Keys), 2);
      Update_Controllers_Net (num_player);
      Update_Frame ();
      Flip ();
    }
#endif
  return 1;
}

int Eff_Screen (void)
{
	int i;
	
	for (i = 0; i < 336 * 240; i++)
		MD_Screen[i] = 0;
	
	return 1;
}

int Pause_Screen (void)
{
  int i, j, offset;
  int r, v, b, nr, nv, nb;

  r = v = b = nr = nv = nb = 0;

  for (offset = j = 0; j < 240; j++)
    {
      for (i = 0; i < 336; i++, offset++)
	{
	  if (Mode_555 & 1)
	    {
	      r = (MD_Screen[offset] & 0x7C00) >> 10;
	      v = (MD_Screen[offset] & 0x03E0) >> 5;
	      b = (MD_Screen[offset] & 0x001F);
	    }
	  else
	    {
	      r = (MD_Screen[offset] & 0xF800) >> 11;
	      v = (MD_Screen[offset] & 0x07C0) >> 6;
	      b = (MD_Screen[offset] & 0x001F);
	    }

	  nr = nv = nb = (r + v + b) / 3;

	  if ((nb <<= 1) > 31)
	    nb = 31;

	  nr &= 0x1E;
	  nv &= 0x1E;
	  nb &= 0x1E;

	  if (Mode_555 & 1)
	    MD_Screen[offset] = (nr << 10) + (nv << 5) + nb;
	  else
	    MD_Screen[offset] = (nr << 11) + (nv << 6) + nb;
	}
    }

  return 1;
}

#ifdef DEBUG_GENESIS
void Show_Genesis_Screen (void)
{
	Do_VDP_Only();
	Flip ();
}
#endif

int Take_Shot(void)
{
#ifndef DEBUG_VDP
	Save_Shot ((unsigned char *) surface->pixels, Mode_555 & 1, surface->w,
		surface->h, surface->w * 2);
#endif
	return 0;
}
