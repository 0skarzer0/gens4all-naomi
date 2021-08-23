#include "gens4all.h"
#include "input.h"

//char Phrase[1024];
static int Nb_Joys = 0;
long MouseX, MouseY;


unsigned char Keys[1024];
#ifdef KAILLERA_SUPPORT
unsigned char Kaillera_Keys[16];
#endif
unsigned char joystate[0x80 + 0x100 + 0x10 * 3 + 0x4];

/* CHUI: Changed key defaults for Dreamcast
struct K_Def Keys_Def[8] = {
  {SDLK_RETURN, SDLK_RSHIFT,
   SDLK_a, SDLK_z, SDLK_e,
   SDLK_q, SDLK_s, SDLK_d,
   SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT},
  {SDLK_u, SDLK_t,
   SDLK_k, SDLK_l, SDLK_m,
   SDLK_i, SDLK_o, SDLK_p,
   SDLK_y, SDLK_h, SDLK_g, SDLK_j},
};
*/
struct K_Def Keys_Def[8] = {
  {SDLK_RETURN, SDLK_SPACE,
   SDLK_LCTRL, SDLK_LALT, SDLK_LSHIFT,
   SDLK_TAB, SDLK_BACKSPACE, SDLK_0,
   SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT},
  {SDLK_z, SDLK_x,
   SDLK_e, SDLK_q, SDLK_c,
   SDLK_1, SDLK_2, SDLK_3,
   SDLK_w, SDLK_s, SDLK_a, SDLK_d},
};

SDL_Joystick *joy[2] = { NULL, NULL };

void
End_Input ()
{
  //close_joystick();
}

void
open_joystick ()
{
  Nb_Joys = 0;
  SDL_JoystickEventState (SDL_ENABLE);
  joy[0] = SDL_JoystickOpen (0);
  if (joy[0])
    Nb_Joys++;
  joy[1] = SDL_JoystickOpen (1);
  if (joy[1])
    Nb_Joys++;
}

void
close_joystick ()
{
  if (SDL_JoystickOpened (0))
    {
      SDL_JoystickClose (joy[0]);
      joy[0] = NULL;
    }
  if (SDL_JoystickOpened (1))
    {
      SDL_JoystickClose (joy[1]);
      joy[1] = NULL;
    }
}

int
Init_Input (void)
{
  memset (joystate, 0, sizeof (joystate));
  if (SDL_NumJoysticks () > 0)
    {
      open_joystick ();
    }
  return 1;
}


void
Restore_Input ()
{
}

int
Check_Key_Pressed (unsigned int key)
{
  int Num_Joy;
  if (key < 1024)
    {
      if (Keys[key])
	return (1);
    }
  else
    {
      Num_Joy = ((key >> 8) & 0xF);

      if ((Num_Joy < 2) && joy[Num_Joy])
	{
	  if (key & 0x80)	// Test POV Joys
	    {
	      switch (key & 0xF)
		{
		case 1:
		  //if (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] == 0) return(1); break;
		  //if (joystate[0x100*Num_Joy].rgdwPOV[(key >> 4) & 3] == 0) return(1); break;
		case 2:
		  //if (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] == 9000) return(1); break;
		  //if (joystate[0x100*Num_Joy].rgdwPOV[(key >> 4) & 3] == 9000) return(1); break;
		case 3:
		  //if (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] == 18000) return(1); break;
		  //if (joystate[0x100*Num_Joy].rgdwPOV[(key >> 4) & 3] == 18000) return(1); break;
		case 4:
		  //if (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] == 27000) return(1); break;
		  //if (joystate[0x100*Num_Joy].rgdwPOV[(key >> 4) & 3] == 27000) return(1); break;
		default:
		  break;
		}

	    }
	  else if (key & 0x70)	// Test Button Joys
	    {
	      if (joystate[0x10 + 0x100 * Num_Joy + (key & 0xFF - 0x10)])
		return 1;
	    }
	  else
	    {
	      switch (key & 0xF)
		{
		case 1:
		  if (joystate[0x100 * Num_Joy + 0x1])
		    return 1;
		  break;

		case 2:
		  if (joystate[0x100 * Num_Joy + 0x2])
		    return 1;
		  break;

		case 3:
		  if (joystate[0x100 * Num_Joy + 0x3])
		    return 1;
		  break;

		case 4:
		  if (joystate[0x100 * Num_Joy + 0x4])
		    return 1;
		  break;
		}
	    }
	}
    }
  return 0;
}

unsigned int Get_Key (void)
{
	SDL_Event sdl_event;
	SDL_Joystick *js[2];
	SDL_JoystickEventState (SDL_ENABLE);
	
	js[0] = SDL_JoystickOpen (0);
	js[1] = SDL_JoystickOpen (1);

	while (SDL_PollEvent (&sdl_event))
	{
	  switch (sdl_event.type)
	    {
	    case SDL_KEYDOWN:
	    	return (unsigned int)sdl_event.key.keysym.sym;
	    case SDL_JOYAXISMOTION:
	      if (sdl_event.jaxis.value < -500)
		{
		  return (0x1000 +
			  (0x100 * sdl_event.jaxis.which +
			   ((sdl_event.jaxis.axis == 0) ? 0x3 : 0x1)));
		}
	      else if (sdl_event.jaxis.value > 500)
		{
		  return (0x1000 +
			  (0x100 * sdl_event.jaxis.which +
			   ((sdl_event.jaxis.axis == 0) ? 0x4 : 0x2)));
		}
	      else if (sdl_event.jaxis.value == 0)
		{
		  return (Get_Key ());
		}
	      break;

	    case SDL_JOYBUTTONUP:
	      return (0x1010 + (0x100 * sdl_event.jbutton.which) +
		      sdl_event.jbutton.button);
	      break;

	      //case SDL_JOYHATMOTION:                                        
		// return(0xdeadbeef + (0x100 * sdl_event.jhat.which) + sdl_event.jhat.hat + sdl_event.jhat.value);
	    }
    }
}

int inkey;
void Update_Controllers(void)
{

	maple_device_t *contr = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
	if (!contr)
		return;
	cont_state_t *cond = (cont_state_t *)maple_dev_status(contr);
	if (!cond)
		return;

	#define CheckMapleButton(condition, var) \
		do { var = !(condition); } while(0)
	CheckMapleButton(cond->buttons & CONT_START, Controller_1_Start);
	CheckMapleButton(cond->buttons & CONT_A, Controller_1_C);
	CheckMapleButton(cond->buttons & CONT_B, Controller_1_B);
	CheckMapleButton(cond->buttons & CONT_X, Controller_1_B);
	CheckMapleButton(cond->buttons & CONT_Y, Controller_1_A);
	CheckMapleButton(cond->buttons & CONT_DPAD_UP, Controller_1_Up);
	CheckMapleButton(cond->buttons & CONT_DPAD_DOWN, Controller_1_Down);
	CheckMapleButton(cond->buttons & CONT_DPAD_LEFT, Controller_1_Left);
	CheckMapleButton(cond->buttons & CONT_DPAD_RIGHT, Controller_1_Right);
#ifdef SKIPLOADER
	if (cond->rtrig > 1 || cond->ltrig > 1) {
#else
	if ((cond->buttons & (CONT_A | CONT_B | CONT_X | CONT_Y | CONT_START)) == (CONT_A | CONT_B | CONT_X | CONT_Y | CONT_START)) {
#endif
		dbgf("Exit\n");
		Gens_Running = 0;
		extern int Reset_Input_Lockout;
		Reset_Input_Lockout = 1;
	}

	inkey = kbd_get_key();
	extern int soundrecenabled;
	extern int soundrecpos;
	if (inkey == '9') {
		dbgf("\nBegin rec\n");
		soundrecenabled = 1;
		soundrecpos = 0;
	}
	if (inkey == '8') {
		#define soundrecbytes	(512*1024)
		extern char soundrec[soundrecbytes];
		int f = open("/pc/home/alex/Emu/Genesis/recdump2.raw", O_WRONLY | O_TRUNC);
		write(f, soundrec, soundrecpos);
		close(f);
		dbgf("Wrote %i\n", soundrecpos);
		
	}
	if (inkey == '5') {
		extern unsigned char Ram_Z80[8*1024];
		Ram_Z80[0x1fff] = 0;
	}
	if (inkey == '6') {
		extern int Last_BUS_REQ_Cnt;
		printf("Input:\n68k PC: %08x\n", M68K_GetPC());
		printf("68k cycles: %i\n", M68K_GetCyclesDone());
		printf("Z80 state: %08x\n", Z80_State);
		printf("Z80 last bus req: %i\n", Last_BUS_REQ_Cnt);
		show_z80_status();
		//~ printf("Z80 PC : %04x\n", z80_get_reg(Z80_REG_PC));
		//~ printf("Z80 AF : %04x\n", z80_get_reg(Z80_REG_AF));	//a is top byte
		//~ printf("Z80 AF2: %04x\n", z80_get_reg(Z80_REG_AF2));
		//~ printf("Z80 HL : %04x\n", z80_get_reg(Z80_REG_HL));
		//~ printf("Z80 HL2: %04x\n", z80_get_reg(Z80_REG_HL2));
		
		int zpc = z80_get_reg(Z80_REG_PC);
		extern unsigned char Ram_Z80[8*1024];
		int i;
		printf("0x1fff: %02x\n", Ram_Z80[0x1fff]);
		for(i = zpc-20; i < (zpc+30); i++) {
			if (i == zpc)
				printf("[");
			printf("%02x ", Ram_Z80[i]);
			if (i == zpc)
				printf("]");
		}
		printf("\n");
		fflush(stdout);
	}
	if (inkey == 'z') {
		extern int raze_clock_counter;
		printf("Clock %i\n", raze_clock_counter);
		show_z80_status();
	}
	if (inkey == 'Q') {
		Set_Frame_Skip (0);
		dbg("No skip");
	}
	if (inkey == 'W') {
		Set_Frame_Skip (-1);
		dbg("Auto skip");
	}
	if (inkey == 'w') {
		extern struct Reg_VDP_Type VDP_Reg;
		
		printf("VWin DOWN: %i\n", VDP_Reg.Win_V_Pos >> 7);
		printf("VWin WVP: %i\n", VDP_Reg.Win_V_Pos & 0x1f);
	}
	if (inkey == 'z') {
		printf("ZS: %i\n", Z80_State);
	}
	if (inkey == 's') {
		Change_Sound();
		dbgf("Sound %i\n", Sound_Enable);
	}
	if (inkey == 'p') {
		printf("Showing profiler\n");
		fflush(stdout);
		dbg("Showing profiler2");
		
		prof_show();
	}
//	if (inkey == 's') {
//		#ifdef NO_SOUND
//			dbg("NO_SOUND defined");
//		#endif
//		dbgf("Sound: En %i Init %i", Sound_Enable, Sound_Initialised)
//	}
	if (inkey == 'c') {
		extern void prof_reset();
		printf("Prof reset\n");
		prof_reset();
	}
	if (inkey == 'r') {
		dbgf("Reset\n");
		Reset_Genesis();
	}
	if (inkey == 'f') {
		int i;
		extern unsigned prof_total;
		
		pvr_stats_t stats;
		pvr_get_stats(&stats);
		printf("Reg time: %i ms\n", stats.reg_last_time);
		printf("Rnd time: %i ms\n", stats.rnd_last_time);
		printf("Vert buff: %i bytes\n", stats.vtx_buffer_used);
		
		dbgf("Last frame CPU load:\n");
		//for(i = 0; i < prof_total; i++) {
		for(i = 1; i < 6; i++) {
			dbgf("%8s: %2.2f ms\n", prof_msg[i], ((float)prof_last_frame_time[i]) / 1000.0f);
		}
		dbgf("Flipwait: %2.2f ms\n", (float)(prof_last_frame_time[PROF_BLIT] - prof_last_frame_time[PROF_VRENDER]) / 1000.0f);
	}
	if (inkey == 'd') {
		int i, algo_type;
		
		if(YM2612.Inter_Step & 0x04000) {
			algo_type = 0;
			printf("Regular\n");
		} else {
			algo_type = 16;
			printf("Interp.\n");
		}
		if(YM2612.LFOinc) {
			printf("LFO enabled\n");
			algo_type |= 8;
		}
		for(i = 0; i < 6; i++) {
			printf("Ch %i: %i\n", i, YM2612.CHANNEL[i].ALGO + algo_type);
		}
	}
/*
	switch(inkey) {
	case 'q':
		Set_Frame_Skip (0);
		break;
	case 'w':
		Set_Frame_Skip (-1);
		break;
	}
//*/
#if 0
	CHECK_PLAYER_PAD (0, 1)
	CHECK_PLAYER_PAD (1, 2)

#ifdef MULTITAP_SUPPORT  
	if (Controller_1_Type & 0x10)	// TEAMPLAYER PORT 1
	{
		CHECK_PLAYER_PAD (2, 1B)
		CHECK_PLAYER_PAD (3, 1C) CHECK_PLAYER_PAD (4, 1D)
	}

	if (Controller_2_Type & 0x10)	// TEAMPLAYER PORT 2
	{
		CHECK_PLAYER_PAD (5, 2B)
		CHECK_PLAYER_PAD (6, 2C) CHECK_PLAYER_PAD (7, 2D)
	}
#endif
#endif
}

#ifdef KAILLERA_SUPPORT
void Scan_Player_Net (int Player)
{
#if 0
  if (!Player)
    return;

  Update_Input ();

  if (Check_Key_Pressed (Keys_Def[0].Up))
    {
      Kaillera_Keys[0] &= ~0x08;
      Kaillera_Keys[0] |= 0x04;
    }
  else
    {
      Kaillera_Keys[0] |= 0x08;
      if (Check_Key_Pressed (Keys_Def[0].Down))
	Kaillera_Keys[0] &= ~0x04;
      else
	Kaillera_Keys[0] |= 0x04;
    }

  if (Check_Key_Pressed (Keys_Def[0].Left))
    {
      Kaillera_Keys[0] &= ~0x02;
      Kaillera_Keys[0] |= 0x01;
    }
  else
    {
      Kaillera_Keys[0] |= 0x02;
      if (Check_Key_Pressed (Keys_Def[0].Right))
	Kaillera_Keys[0] &= ~0x01;
      else
	Kaillera_Keys[0] |= 0x01;
    }

  if (Check_Key_Pressed (Keys_Def[0].Start))
    Kaillera_Keys[0] &= ~0x80;
  else
    Kaillera_Keys[0] |= 0x80;

  if (Check_Key_Pressed (Keys_Def[0].A))
    Kaillera_Keys[0] &= ~0x40;
  else
    Kaillera_Keys[0] |= 0x40;

  if (Check_Key_Pressed (Keys_Def[0].B))
    Kaillera_Keys[0] &= ~0x20;
  else
    Kaillera_Keys[0] |= 0x20;

  if (Check_Key_Pressed (Keys_Def[0].C))
    Kaillera_Keys[0] &= ~0x10;
  else
    Kaillera_Keys[0] |= 0x10;

  if (Controller_1_Type & 1)
    {
      if (Check_Key_Pressed (Keys_Def[0].Mode))
	Kaillera_Keys[1] &= ~0x08;
      else
	Kaillera_Keys[1] |= 0x08;

      if (Check_Key_Pressed (Keys_Def[0].X))
	Kaillera_Keys[1] &= ~0x04;
      else
	Kaillera_Keys[1] |= 0x04;

      if (Check_Key_Pressed (Keys_Def[0].Y))
	Kaillera_Keys[1] &= ~0x02;
      else
	Kaillera_Keys[1] |= 0x02;

      if (Check_Key_Pressed (Keys_Def[0].Z))
	Kaillera_Keys[1] &= ~0x01;
      else
	Kaillera_Keys[1] |= 0x01;
    }
#endif
}

void Update_Controllers_Net (int num_player)
{
#if 0
  Controller_1_Up = (Kaillera_Keys[0] & 0x08) >> 3;
  Controller_1_Down = (Kaillera_Keys[0] & 0x04) >> 2;
  Controller_1_Left = (Kaillera_Keys[0] & 0x02) >> 1;
  Controller_1_Right = (Kaillera_Keys[0] & 0x01);
  Controller_1_Start = (Kaillera_Keys[0] & 0x80) >> 7;
  Controller_1_A = (Kaillera_Keys[0] & 0x40) >> 6;
  Controller_1_B = (Kaillera_Keys[0] & 0x20) >> 5;
  Controller_1_C = (Kaillera_Keys[0] & 0x10) >> 4;

  if (Controller_1_Type & 1)
    {
      Controller_1_Mode = (Kaillera_Keys[0 + 1] & 0x08) >> 3;
      Controller_1_X = (Kaillera_Keys[0 + 1] & 0x04) >> 2;
      Controller_1_Y = (Kaillera_Keys[0 + 1] & 0x02) >> 1;
      Controller_1_Z = (Kaillera_Keys[0 + 1] & 0x01);
    }

  if (num_player > 2)		// TEAMPLAYER
    {
      Controller_1B_Up = (Kaillera_Keys[2] & 0x08) >> 3;
      Controller_1B_Down = (Kaillera_Keys[2] & 0x04) >> 2;
      Controller_1B_Left = (Kaillera_Keys[2] & 0x02) >> 1;
      Controller_1B_Right = (Kaillera_Keys[2] & 0x01);
      Controller_1B_Start = (Kaillera_Keys[2] & 0x80) >> 7;
      Controller_1B_A = (Kaillera_Keys[2] & 0x40) >> 6;
      Controller_1B_B = (Kaillera_Keys[2] & 0x20) >> 5;
      Controller_1B_C = (Kaillera_Keys[2] & 0x10) >> 4;

      if (Controller_1B_Type & 1)
	{
	  Controller_1B_Mode = (Kaillera_Keys[2 + 1] & 0x08) >> 3;
	  Controller_1B_X = (Kaillera_Keys[2 + 1] & 0x04) >> 2;
	  Controller_1B_Y = (Kaillera_Keys[2 + 1] & 0x02) >> 1;
	  Controller_1B_Z = (Kaillera_Keys[2 + 1] & 0x01);
	}

      Controller_1C_Up = (Kaillera_Keys[4] & 0x08) >> 3;
      Controller_1C_Down = (Kaillera_Keys[4] & 0x04) >> 2;
      Controller_1C_Left = (Kaillera_Keys[4] & 0x02) >> 1;
      Controller_1C_Right = (Kaillera_Keys[4] & 0x01);
      Controller_1C_Start = (Kaillera_Keys[4] & 0x80) >> 7;
      Controller_1C_A = (Kaillera_Keys[4] & 0x40) >> 6;
      Controller_1C_B = (Kaillera_Keys[4] & 0x20) >> 5;
      Controller_1C_C = (Kaillera_Keys[4] & 0x10) >> 4;

      if (Controller_1C_Type & 1)
	{
	  Controller_1C_Mode = (Kaillera_Keys[4 + 1] & 0x08) >> 3;
	  Controller_1C_X = (Kaillera_Keys[4 + 1] & 0x04) >> 2;
	  Controller_1C_Y = (Kaillera_Keys[4 + 1] & 0x02) >> 1;
	  Controller_1C_Z = (Kaillera_Keys[4 + 1] & 0x01);
	}

      Controller_1D_Up = (Kaillera_Keys[6] & 0x08) >> 3;
      Controller_1D_Down = (Kaillera_Keys[6] & 0x04) >> 2;
      Controller_1D_Left = (Kaillera_Keys[6] & 0x02) >> 1;
      Controller_1D_Right = (Kaillera_Keys[6] & 0x01);
      Controller_1D_Start = (Kaillera_Keys[6] & 0x80) >> 7;
      Controller_1D_A = (Kaillera_Keys[6] & 0x40) >> 6;
      Controller_1D_B = (Kaillera_Keys[6] & 0x20) >> 5;
      Controller_1D_C = (Kaillera_Keys[6] & 0x10) >> 4;

      if (Controller_1D_Type & 1)
	{
	  Controller_1D_Mode = (Kaillera_Keys[6 + 1] & 0x08) >> 3;
	  Controller_1D_X = (Kaillera_Keys[6 + 1] & 0x04) >> 2;
	  Controller_1D_Y = (Kaillera_Keys[6 + 1] & 0x02) >> 1;
	  Controller_1D_Z = (Kaillera_Keys[6 + 1] & 0x01);
	}
    }
  else
    {
      Controller_2_Up = (Kaillera_Keys[2] & 0x08) >> 3;
      Controller_2_Down = (Kaillera_Keys[2] & 0x04) >> 2;
      Controller_2_Left = (Kaillera_Keys[2] & 0x02) >> 1;
      Controller_2_Right = (Kaillera_Keys[2] & 0x01);
      Controller_2_Start = (Kaillera_Keys[2] & 0x80) >> 7;
      Controller_2_A = (Kaillera_Keys[2] & 0x40) >> 6;
      Controller_2_B = (Kaillera_Keys[2] & 0x20) >> 5;
      Controller_2_C = (Kaillera_Keys[2] & 0x10) >> 4;

      if (Controller_2_Type & 1)
	{
	  Controller_2_Mode = (Kaillera_Keys[2 + 1] & 0x08) >> 3;
	  Controller_2_X = (Kaillera_Keys[2 + 1] & 0x04) >> 2;
	  Controller_2_Y = (Kaillera_Keys[2 + 1] & 0x02) >> 1;
	  Controller_2_Z = (Kaillera_Keys[2 + 1] & 0x01);
	}
    }
#endif
}
#endif
