/*****************************************************************************************/
/* Gens4ALL: Main C module                                                               */
/*****************************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <SDL.h>
#include "gens4all.h"
#include "misc.h"
#include "util/SFont.h"
#include "menu/menu.h"
#include "pvrvdp/profile.h"
#include "pvrvdp/watchdog.h"
#include "gamelist.h"


#include <dc/sound/sound.h>
#include <dc/sound/aica_comm.h>


#ifdef DREAMCAST
#include <kos.h>
extern uint8 romdisk[];
//~ KOS_INIT_FLAGS(INIT_DEFAULT | INIT_OCINDEX);
KOS_INIT_FLAGS(INIT_DEFAULT);
KOS_INIT_ROMDISK(romdisk);
#endif

#ifdef DEBUG_GENS4ALL
#if !defined(START_DEBUG) || START_DEBUG==0
int debuggingnow=1;
#else
int debuggingnow=0;
#endif
#ifdef DEBUG_FILE
FILE *DEBUG_STR_FILE=NULL;
#endif
#endif


static int fast_forward = 0;

POINT Window_Pos;
char Str_Tmp[1024];
char Gens_Path[1024];
char Language_Path[1024];
char CGOffline_Path[1024];
char Manual_Path[1024];

char **language_name = NULL;
int Active = 1;
int Paused = 0;
int Net_Play = 0;
int Full_Screen = -1;
int Resolution = 1;
int Fast_Blur = 0;
int Render_W = 0;
int Render_FS = 0;
//~ int Show_FPS = 0;
int Show_Message = 0;
int Show_LED = 0;
int FS_Minimised = 0;
int Auto_Pause = 0;
int Auto_Fix_CS = 0;
int Language = 0;
int Country = -1;
int Country_Order[3];
int Intro_Style = 2;
int SegaCD_Accurate = 0;
int Kaillera_Client_Running = 0;
int SegaCD_Started=0;
int _32X_Started=0;

SFont_Font* Font;

int Gens_Running = 0;

unsigned long long prof_last_frame_time[GENS4ALL_PROFILER_MAX];
char *prof_msg[GENS4ALL_PROFILER_MAX];
float last_frame_busy;
float frame_busy_avrg;
unsigned long long render_total_us = 16000;
#ifdef PROFILER_GENS4ALL
static unsigned prof_total_initial=0;
unsigned prof_total=0;
char *prof_msg[GENS4ALL_PROFILER_MAX];
G4AProfiler g4a_profiler;
unsigned long long prof_initial[GENS4ALL_PROFILER_MAX];	//microseconds
unsigned long long prof_sum[GENS4ALL_PROFILER_MAX];
unsigned long long prof_executed[GENS4ALL_PROFILER_MAX];
unsigned long long prof_peak[GENS4ALL_PROFILER_MAX];
unsigned long long prof_peak_decay[GENS4ALL_PROFILER_MAX];
int prof_started[GENS4ALL_PROFILER_MAX];
int prof_frames;


unsigned long long prof_frame_sum[GENS4ALL_PROFILER_MAX];

long long last_frame_start_time_us = 0;
long long last_frame_duration_us = 16000;


void prof_reset(void)
{
	int i;
	unsigned long long s=get_time_in_micros();
	for(i=0;i<GENS4ALL_PROFILER_MAX;i++)
	{
		prof_initial[i]=s;
		prof_sum[i]=0;
		if (prof_started[i])
			prof_executed[i]=1;
		else
			prof_executed[i]=0;
	}
	prof_total_initial=s;
}

void prof_init(void)
{
	unsigned i;
	unsigned long long s=get_time_in_micros();
	//i = 1 in order to not reset main thread prof
	for(i=1;i<GENS4ALL_PROFILER_MAX;i++)
	{
		prof_initial[i]=s;
		prof_sum[i]=0;
		if (!prof_total)
		{
			prof_executed[i]=0;
			prof_started[i]=0;
			prof_msg[i]=NULL;
		}
		else
		{
			if (prof_started[i])
			        prof_executed[i]=1;
			else
			        prof_executed[i]=0;
		}
	}
	prof_total_initial=s;
}

void prof_add(char *msg)
{
	if (prof_total<GENS4ALL_PROFILER_MAX)
	{
		prof_msg[prof_total]=msg;
		prof_total++;
	}
}

void prof_show(void)
{
	int i;
	//~ dbg("Pro_show called");
	float totaltime = 0;
	double toper=0;
	unsigned long long to;
//	for(i=prof_total-1;i>=0;i--)
//		prof_end(i);
	to=prof_sum[0];
	//~ puts("\n");
	puts("--------------------------------------------");
	for(i=0;i<prof_total;i++)
	{
		if (0) {
			unsigned long long t0=prof_sum[i];
			double percent=(double)t0;
			percent*=100.0;
			percent/=(double)to;
			toper+=percent;
	#ifndef PROFILER_SDL
			t0/=1000;
	#endif
			float milliseconds = (float)t0;
			
			printf("%s: %.2f%% -> Ticks=%i ->",prof_msg[i],percent,((unsigned)t0));
			if (prof_executed[i]>3000LL) {
				printf("%.2fK times",(float)prof_executed[i]/1000.0f);
				//printf("%iK times\n",(unsigned)(prof_executed[i]/1000LL));
			} else {
				printf("%i times",(unsigned)(prof_executed[i]));
			}
			printf("  | %f ms per frame\n", milliseconds / prof_frames);
		} else {
			float timems = prof_last_frame_time[i] / 1000.0f;
			totaltime += timems;
			printf("%s: %.2f ms\n",prof_msg[i], timems);
		}
		
	}
	printf("Total: %f ms\n", totaltime);
	printf("Total: %f ms (minus render) \n", totaltime-(prof_last_frame_time[PROF_VRENDER]+prof_last_frame_time[PROF_PVRWAIT])/1000.0f);
	//~ printf("TOTAL: %.2f%% -> Ticks=%i | frames %i\n",toper,to,prof_frames);
	puts("--------------------------------------------\n"); fflush(stdout);
	
	prof_reset();
}

void prof_setmsg(int n, char *msg)
{
	prof_msg[n]=msg;
}

#endif


static void clear_SDL_events(void)
{
  SDL_Event event;
  while (SDL_PollEvent (&event))
	  SDL_Delay(10);
}

int update_SDL_events ()
{
#if !defined(DEBUG_GENS4ALL) && !defined(PROFILER_GENS4ALL)
  static int toexit=0;
  SDL_Event event;

  SDLMod mod;
  if (toexit)
	    {
		              if (toexit>33)
				                {
							                toexit=0;
									                return -1;
											          }
			                else
						                  toexit++;
					  }

  while (SDL_PollEvent (&event))
    {
      switch (event.type)
	{
	  /*
	     case SDL_VIDEORESIZE:
	     surface = SDL_SetVideoMode(event.resize.w, event.resize.h, 16, SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_RESIZABLE);
	     break;
	   */
	case SDL_QUIT:
		return -1;
	case SDL_KEYDOWN:
	  if (event.key.keysym.sym==SDLK_ESCAPE || event.key.keysym.sym==SDLK_BACKSPACE)
		  toexit=1;
	  else
	  Keys[event.key.keysym.sym] = 1;
// CHUI: Disabled (needs a menu)
#if 0
	  mod = SDL_GetModState ();

	  switch (event.key.keysym.sym)
	    {
	    case SDLK_ESCAPE:
	      if (Debug)
		{
		  Change_Debug (0);
		  Paused = 0;
		}
	      else if (Paused)
		{
		  Paused = 0;
		}
	      else
		{
		  Paused = 1;
		  Pause_Screen ();
		  Clear_Sound_Buffer ();
		}
	      break;

	    case SDLK_BACKSPACE:
	      if (KMOD_SHIFT & mod)
		{
		  Clear_Sound_Buffer ();
		  Take_Shot ();
		}
	      break;

	    case SDLK_TAB:
	      system_reset ();
	      break;

	    case SDLK_RETURN:
	      if (KMOD_ALT & mod)
		{
		  if (Full_Screen)
		    Set_Render (0, -1, 1);
		  else
		    Set_Render (1, Render_FS, 1);
		}
	      break;

	    case SDLK_F1:
	      fast_forward = 1;
	      break;

	    case SDLK_F2:
	      if (KMOD_SHIFT & mod)
		{
		  Change_Stretch ();
		  sync_gens_ui ();
		}
	      else if (!mod)
		{
		  Set_Frame_Skip (-1);
		  sync_gens_ui ();
		}
	      break;

	    case SDLK_F3:
	      if (KMOD_SHIFT & mod)
		{
		  Change_VSync ();
		  sync_gens_ui ();
		}
	      else if (!mod)
		{
		  if (Frame_Skip == -1)
		    {
		      Set_Frame_Skip (0);
		      sync_gens_ui ();
		    }
		  else if (Frame_Skip > 0)
		    {
		      Set_Frame_Skip (Frame_Skip - 1);
		      sync_gens_ui ();
		    }
		}
	      break;


	    case SDLK_F4:
	      if (Frame_Skip == -1)
		{
		  Set_Frame_Skip (1);
		}
	      else
		{
		  if (Frame_Skip < 8)
		    Set_Frame_Skip (Frame_Skip + 1);
		}
	      sync_gens_ui ();
	      break;

	    case SDLK_F5:
	      if (KMOD_SHIFT & mod)
		{
		  //if (Check_If_Kaillera_Running()) return 0;
		  MINIMIZE Change_File_S (Str_Tmp, State_Dir);
		  Save_State (Str_Tmp);
		}
	      else if (!mod)
		{
		  //if (Check_If_Kaillera_Running()) return 0;
		  Str_Tmp[0] = 0;
		  Get_State_File_Name (Str_Tmp);
		  Save_State (Str_Tmp);
		}
	      break;

	    case SDLK_F6:
	      Set_Current_State ((Current_State + 9) % 10);
	      sync_gens_ui ();
	      break;

	    case SDLK_F7:
	      Set_Current_State ((Current_State + 1) % 10);
	      sync_gens_ui ();
	      break;

	    case SDLK_F8:
	      if (KMOD_SHIFT & mod)
		{
		  //if (Check_If_Kaillera_Running()) return 0;
		  MINIMIZE Str_Tmp[0] = 0;
		  Change_File_L (Str_Tmp, State_Dir);
		  Load_State (Str_Tmp);
		}
	      else if (!mod)
		{
		  //if (Check_If_Kaillera_Running()) return 0;
		  Str_Tmp[0] = 0;
		  Get_State_File_Name (Str_Tmp);
		  Load_State (Str_Tmp);
		}
	      break;

	    case SDLK_F9:
	      if (KMOD_SHIFT & mod)
		{
		  Change_Blit_Style ();
		}
	      else if (!mod)
		{
		  Change_Fast_Blur ();
		}
	      break;

	    case SDLK_F10:
	      if (KMOD_SHIFT & mod)
		{
		  Change_DAC_Improv ();
		  sync_gens_ui ();
		}
	      else if (!mod)
		{
		  ShDEBUG_GENS4ALLow_FPS = !Show_FPS;
		}
	      break;

	    case SDLK_F11:
	      if (KMOD_SHIFT & mod)
		{
		  Change_PSG_Improv ();
		  sync_gens_ui ();
		}
	      else if (!mod)
		{
		  if (Full_Screen && (Render_FS > 0))
		    {
		      Set_Render (1, Render_FS - 1, 0);
		      sync_gens_ui ();
		    }
		  else if (!Full_Screen && (Render_W > 0))
		    {
		      Set_Render (0, Render_W - 1, 0);
		      sync_gens_ui ();
		    }
		}
	      break;

	    case SDLK_F12:
	      if (KMOD_SHIFT & mod)
		{
		  Change_YM2612_Improv ();
		  sync_gens_ui ();
		}
	      else if (!mod)
		{
		  if (Full_Screen && (Render_FS < NB_FILTER))
		    {
		      Set_Render (1, Render_FS + 1, 0);
		      sync_gens_ui ();
		    }
		  else if (!Full_Screen && (Render_W < NB_FILTER))
		    {
		      Set_Render (0, Render_W + 1, 0);
		      sync_gens_ui ();
		    }
		}
	      break;

	    case SDLK_0:
	      if (KMOD_SHIFT & mod)
		{
		  Set_Current_State (0);
		  sync_gens_ui ();
		}
	      break;

	    case SDLK_1:
	    case SDLK_2:
	    case SDLK_3:
	    case SDLK_4:
	    case SDLK_5:
	    case SDLK_6:
	    case SDLK_7:
	    case SDLK_8:
	    case SDLK_9:
	      if (KMOD_SHIFT & mod)
		{
		  Set_Current_State (event.key.keysym.sym - SDLK_0);
		  sync_gens_ui ();
		}
	      else if (KMOD_CTRL & mod)
		{
		  //if ((Check_If_Kaillera_Running())) return 0;
		  if (GYM_Playing)
		    Stop_Play_GYM ();
		  Pre_Load_Rom (Recent_Rom[event.key.keysym.sym - SDLK_0]);
		  sync_gens_ui ();
		}
	      break;

	    case SDLK_b:
	      if (KMOD_CTRL & mod)
		{
#ifdef SEGA_CD
		  if (Num_CD_Drive == 0)
		    return;	// return 1;
#endif

		  //if (Check_If_Kaillera_Running()) return 0;
		  if (GYM_Playing)
		    Stop_Play_GYM ();
		  Free_Rom (Game);	// Don't forget it !
		  #ifdef SEGA_CD
		  SegaCD_Started = Init_SegaCD (NULL);
		  #endif
		}
	      break;

	    case SDLK_c:
	      if (KMOD_CTRL & mod)
		{
		  if (Sound_Initialised)
		    Clear_Sound_Buffer ();
		  Debug = 0;
		  if (Net_Play)
		    {
		      if (Full_Screen)
			Set_Render (0, -1, 1);
		      sync_gens_ui ();
		    }
		  Free_Rom (Game);
		}
	      break;

	    case SDLK_g:
	      if (KMOD_CTRL & mod)
		{
		  //if (Check_If_Kaillera_Running()) return 0;
		  MINIMIZE open_game_genie ();
		}
	      break;

	    case SDLK_o:
	      if (KMOD_CTRL & mod)
		{
		  //if ((Check_If_Kaillera_Running())) return 0;
		  if (GYM_Playing)
		    Stop_Play_GYM ();
//		  MINIMIZE Get_Rom ();
		}
	      break;

	    case SDLK_p:
	      if ((KMOD_CTRL | KMOD_SHIFT) & mod)
		{
		  if (!Genesis_Started && !SegaCD_Started && !_32X_Started)
		    {
		      MINIMIZE if (GYM_Playing)
			Stop_Play_GYM ();
		      else
			Start_Play_GYM ();
		      sync_gens_ui ();
		    }
		}
	      else if (KMOD_CTRL & mod)
		{
#ifdef SEGA_CD
		  Change_SegaCD_Synchro ();
#endif
		  sync_gens_ui ();
		}

	      break;

	    case SDLK_v:
#ifdef SEGA_CD
	      if (KMOD_CTRL & mod)
		{
		  if (SegaCD_Started)
		    Change_CD ();
		}
#endif
	      break;

	    case SDLK_w:
	      if (KMOD_CTRL & mod)
		{
		  if (WAV_Dumping)
		    Stop_WAV_Dump ();
		  else
		    Start_WAV_Dump ();
		  sync_gens_ui ();
		}
	      break;

	    default:
	      break;
	    }
#endif
	  break;

	case SDL_KEYUP:
	  if (event.key.keysym.sym==SDLK_ESCAPE || event.key.keysym.sym==SDLK_BACKSPACE)
		  toexit=1;
	  else
	  Keys[event.key.keysym.sym] = 0;
	  if (event.key.keysym.sym == SDLK_F1)
	    {
	      fast_forward = 0;
	    }
	  break;

	case SDL_JOYAXISMOTION:
	  if (event.jaxis.axis < 2)
	    {
	      if (event.jaxis.value < -500)
		{
		  joystate[0x100 * event.jaxis.which +
			   ((event.jaxis.axis == 0) ? 0x3 : 0x1)] = 1;
		  joystate[0x100 * event.jaxis.which +
			   ((event.jaxis.axis == 0) ? 0x4 : 0x2)] = 0;
		}
	      else if (event.jaxis.value > 500)
		{
		  joystate[0x100 * event.jaxis.which +
			   ((event.jaxis.axis == 0) ? 0x4 : 0x2)] = 1;
		  joystate[0x100 * event.jaxis.which +
			   ((event.jaxis.axis == 0) ? 0x3 : 0x1)] = 0;
		}
	      else
		{
		  if (event.jaxis.axis == 0)
		    {
		      joystate[0x100 * event.jaxis.which + 0x4] = 0;
		      joystate[0x100 * event.jaxis.which + 0x3] = 0;
		    }
		  else if (event.jaxis.axis == 1)
		    {
		      joystate[0x100 * event.jaxis.which + 0x2] = 0;
		      joystate[0x100 * event.jaxis.which + 0x1] = 0;
		    }

		}
	    }

	  break;

#ifndef DREAMCAST
	case SDL_JOYBUTTONDOWN:
	  joystate[0x10 + 0x100 * event.jbutton.which +
		   event.jbutton.button] = 1;
	  break;

	case SDL_JOYBUTTONUP:
	  joystate[0x10 + 0x100 * event.jbutton.which +
		   event.jbutton.button] = 0;
	  break;

	case SDL_JOYHATMOTION:
	  break;
#endif

	default:
	  break;
	}
    }
#else
 	static Uint32 nevents=0;

	switch (nevents&7)
	{
		case 1:
	  		Keys[SDLK_RETURN] = 1;
			break;
		case 2:
	  		Keys[SDLK_RETURN] = 0;
			break;
		case 4:
	  		Keys[SDLK_LCTRL] = 1;
			break;
		case 5:
	  		Keys[SDLK_LCTRL] = 0;
			break;
	}
	//dbgf("--- EVENT %i ---\n",nevents);
	nevents++;
#ifdef DEBUG_GENS4ALL
#ifdef START_DEBUG
	if (nevents>START_DEBUG)
	{
#ifdef DEBUG_FILE
		if (!DEBUG_STR_FILE)
			DEBUG_STR_FILE=fopen(DEBUG_FILE,"wt");
#endif
		debuggingnow=1;
	}
#endif
#ifdef END_DEBUG
	if (nevents>END_DEBUG)
	{
#ifdef DEBUG_FILE
		fclose(DEBUG_STR_FILE);
		SDL_Delay(333);
#endif
		exit(0);
	}
#endif
#else
#ifdef PROFILER_GENS4ALL
#ifdef RESET_PROFILER
	if (nevents==RESET_PROFILER)
		prof_reset();
#endif
#ifdef END_PROFILER
	if (nevents>END_PROFILER)
	{
		prof_show();
		exit(0);
	}
#endif
#endif
#endif
#endif
	return 0;
}

#if 0
INLINE void close_gens ()
{
  Gens_Running = 0;
}

INLINE void run_gens ()
{
  Gens_Running = 1;
}
#endif

INLINE int is_gens_running ()
{
  return Gens_Running;
}

static int Build_Language_String (void)
{
	unsigned long nb_lue = 1;
	int sec_alloue = 1, poscar = 0;
	enum etat_sec
	{ DEB_LIGNE, SECTION, NORMAL } etat = DEB_LIGNE;

	FILE *LFile;
	char c;

	if (language_name)
	{
		free (language_name);
		language_name = NULL;
	}

	language_name = (char **) malloc (sec_alloue * sizeof (char *));
	language_name[0] = NULL;

	language_name = (char **) realloc (language_name, 2 * sizeof (char *));
	language_name[0] = (char *) malloc (32 * sizeof (char));
	strcpy (language_name[0], "English");
	language_name[1] = NULL;

	return (0);
}


int Init (void)
{
	//memset(MD_Screen, 0, 336 * 240 * 2);
	if (SDL_Init(/*SDL_INIT_VIDEO|*/SDL_INIT_JOYSTICK|SDL_INIT_CDROM
#ifndef NO_SOUND
		| SDL_INIT_AUDIO
#endif
		) < 0)
	{
		fprintf (stderr, SDL_GetError ());
		return 0;
	}

	dbg("InitFont");
	// Iniciar SFont
	Font = SFont_InitFont((SDL_Surface *)SDL_LoadBMP(DATA_PREFIX "sfonts.bmp"));
	if (!Font)
	{
		fprintf(stderr, "An error occured while loading the font.");
		return 0;
	}

	// Esto debe servir para la medicion de los fps durante el juego
	// habria que implementar algunas funciones que no dependan de la plataforma
	// los porters del Gens para Linux hicieron uso de la misma
  //init_timer ();

	dbg("Reset variables");
	Net_Play = 0;
	Full_Screen = -1;
	VDP_Num_Vis_Lines = 224;
	Resolution = 1;
	W_VSync = 0;
	FS_VSync = 0;
	Stretch = 0;
	Sprite_Over = 1;
	Render_W = 0;
	Render_FS = 0;
	Show_Message = 1;
	
	Sound_Enable = 1;
	Sound_Segs = 8;
	Sound_Stereo = 1;
	//~ Sound_Initialised = 1;	//Sound not inited yet...
	Sound_Is_Playing = 0;
	WAV_Dumping = 0;
	GYM_Dumping = 0;
	
	FS_Minimised = 0;
	Game = NULL;
	Genesis_Started = 0;
	SegaCD_Started = 0;
	_32X_Started = 0;
	Debug = 0;
	CPU_Mode = 0;

#ifdef ARCH_X86
	Identify_CPU ();
#endif

	strcpy(Gens_Path, "./");

	strncpy (Language_Path, Gens_Path, 1024);
	strcat (Language_Path, "/language.dat");
	
	strncpy (Str_Tmp, Gens_Path, 1024);
	strcat (Str_Tmp, "/gens.cfg");
	
	strcpy (Manual_Path, "");
	strcpy (CGOffline_Path, "");
	
	Build_Language_String ();

	dbg("Init CPUs");
#ifdef SEGA_32X
	MSH2_Init ();
	SSH2_Init ();
#endif

	M68K_Init ();

#ifdef SEGA_CD
	S68K_Init ();
#endif

	Z80_Init ();
	YM2612_Init (CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
	PSG_Init (CLOCK_NTSC / 15, Sound_Rate);

#ifdef SEGA_32X
	PWM_Init ();
#endif

	dbg("Load config");
	Load_Config (Str_Tmp, NULL);
	//Sound_Enabled = 1;
	Init_Input ();

#ifdef SEGA_CD
	Init_CD_Driver ();
#endif

	Init_Tab ();
	run_gens ();

	return 1;
}

void End_All (void)
{
	printf("End_All called\n");
	fflush(stdout);
	Free_Rom(Game);
	Shutdown_Video_SubSystem();
	End_Input();
	YM2612_End();
	End_Sound();
	
#ifdef SEGA_CD
	End_CD_Driver();
#endif
	
	SDL_Quit();
}


#include "../kernel/arch/dreamcast/hardware/pvr/pvr_internal.h"
#include <arch/irq.h>
//irq_context_t breakpcprinter;
//volatile int breakpc;
void break_handler(irq_t source, irq_context_t *context) {
	(void)source;

	int breakpc = CONTEXT_PC(*context);
	printf("***************Break at %08x***************\n", breakpc);
	fflush(stdout);
	ubc_clear_breakpoints();	/* KOS nueva: ubc_disable_all() ya no existe */
}

static volatile int sleeptime = 0;
static volatile int paused = 0;
static kthread_t * dogthd = NULL;
static kthread_t * parentthd = NULL;
static void * watchdog(void *v) {
	extern int raze_clock_counter;
	
	do {
		thd_sleep(26);
		
		extern void pfSampRecordSample(void);
		if (pfSampIsRunning())
			pfSampRecordSample();
		
		if (!paused) {
			sleeptime++;
			if (sleeptime == 60) {
				printf("At PC: %08X, PR: %08X\n",(unsigned int)parentthd->context.pc,(unsigned int)parentthd->context.pr);
				printf("Z80 Clock %i\n", raze_clock_counter);
				int i;
				for(i = 0; i < 16; i++) {
					printf("R%i: %08X  ", i, (unsigned int)parentthd->context.r[i]);
					if ((i % 4) == 3)
						printf("\n");
				}
				printf("\n");
			}
			if (sleeptime == 200) {
				printf("At PC: %08X, PR: %08X\n",(unsigned int)parentthd->context.pc,(unsigned int)parentthd->context.pr);
				printf("Z80 Clock %i\n", raze_clock_counter);
				int i;
				for(i = 0; i < 16; i++) {
					printf("R%i: %08X  ", i, (unsigned int)parentthd->context.r[i]);
					if ((i % 4) == 3)
						printf("\n");
				}
				printf("\n");
				fflush(stdout);
				//~ arch_panic("Watchdog timeout\n");
			}
		}
	} while (1);
}
void watchdog_pet()
{
	sleeptime = 0;
	wdPet();
}
void watchdog_pause()
{
	paused = 1;
	wdPause();
}
void watchdog_resume()
{
	paused = 0;
	wdResume();
}
void watchdog_init()
{
	parentthd = thd_current;
	dogthd = thd_create(0, &watchdog, NULL);
}



int Sonic_2_Auto_Render = 0;
int Sonic_SMPS_Z80_Hack = 0;
int Ristar_Hack = 0;
int Force_Screen_On_Hack = 0;
int Render_Point = 0;	//0 render at start of vblank, 1 render at end of vblank, 2 render at first screen off

int main (int argc, char *argv[])
{
	MenuEntry menu_entries[] = 
	{
		{"Selector de archivos", FILESEL_ITEM, NULL, ".", NULL, 1},
		{"Seleccion de controles", ACTION_ITEM, NULL, NULL, NULL, 0},
/*		{"Cargar configuracion", ACTION_ITEM, NULL, NULL, NULL, 1},
		{"Guardar configuracion", ACTION_ITEM, NULL, NULL, NULL, 0},*/
		{"Opciones de sonido", ACTION_ITEM, NULL, NULL, NULL, 0},
		{"Opciones de video", ACTION_ITEM, NULL, NULL, NULL, 0},
		{"Sonido", BOOLEAN_ITEM, NULL, NULL, &Sound_Enable, 1},
//		{"Frameskip", NUM_VALUE_ITEM, NULL, NULL, &gens_frameskip, 1},
		{"Cerrar emulador", ACTION_ITEM, End_All, NULL, NULL, 1},
		{"", 0, NULL, NULL, NULL, 0}
	};

	MenuType menu;

	menu.entries = menu_entries;
	menu.line_spacing  = 4;

#ifdef DREAMCAST
	char *nargv[]={ "gens4all", ROM_PREFIX "example.bin.gz" };
    	puts("MAIN !!!!");
	argv=nargv;
#else
	if (argc < 2)
	{
		perror("Gens - Genesis emulator by Stèphane Dallongueville");
		fprintf(stderr,"Usage: %s <rom file>\n",argv[0]);
		return 1;
	}
#endif

	dbg("Init genesis bios");
	Rom_Data=(unsigned char *)calloc(6 ,1024 * 1024);
//	memset((void *)VRam,0,sizeof(VRam));
//	memset((void *)CRam,0,sizeof(CRam));
//	memset((void *)VSRam_Over,0,sizeof(VSRam_Over));
//	memset((void *)H_Counter_Table,0,sizeof(H_Counter_Table));
	Init_Genesis_Bios ();

#ifdef PROFILER_GENS4ALL
	prof_init();
	prof_add("Main");           // PROF_TOTAL
	prof_add("M68k");           // PROF_M68K
	prof_add("Z80");            // PROF_Z80
	prof_add("Yamaha");         // PROF_YMH
	prof_add("Blit");           // PROF_BLIT
	prof_add("VRender");        // PROF_VRENDER
	prof_add("SRender");        // PROF_SRENDER
	prof_add("MainLoop");       // PROF_MAINLOOP
	prof_add("PVRWait");        // PROF_PVRWAIT
	prof_add("PROF_MAPA");             // PROF_R2
	prof_add("PROF_MAPB");             // PROF_R3
	prof_add("PROF_SPR");             // PROF_R4
	prof_add("PROF_TEX");             // PROF_R5
	prof_add("R6");             // PROF_R6
	prof_add("R7");             // PROF_R7
#endif

	
	if (!Init ())
		return 0;

#if !defined(DREAMCAST) && !defined(DEBUG_VDP)
	setWindowIcon(DATA_PREFIX "gens_ico.bmp");
#endif

#if defined(DEBUG_GENS4ALL) || defined(PROFILER_GENS4ALL)
	Set_Frame_Skip (0);
#endif
	sync_gens_ui ();
	Set_Frame_Skip (-1);

/*
	dbg("Get rom");
	if (Get_Rom(argv[1]) == -1)
	{
		fprintf(stderr,"Error loading ROM file\n");
		return 1;
	}
*/

	clear_SDL_events();
	dbg("Main loop");
	//menu_init(surface,&menu,"data/");
	//menu_run();
	
	//~ Sound_Rate = 44100;
	//~ Sound_Rate = 32000;
	//~ Sound_Rate = 16000;
	//~ Sound_Rate = 11025;
	//~ Sound_Rate = 8000;
	//~ YM2612_Improv = 1;
	Sound_Enable = 1;
	//~ DAC_Enable = 1;
	Z80_State = 1;
	//~ printf("Using list   : %i\n", PVRVDP_TARGET_LIST);
	printf("Sound Rate   : %i\n", Sound_Rate);
	printf("Sound Enabled: %i\n", Sound_Enable);
	printf("DAC Enabled  : %i\n", DAC_Enable);
	printf("Z80 State    : %i\n", Z80_State);
	fflush(stdout);
	
	//irq_set_handler(EXC_USER_BREAK_PRE, &break_handler);
	//ubc_break_data_write(((uint32)&pvr_state.recording)+0);
	//ubc_break_data_write((uint32)&Gens_Running);
	
	
	watchdog_init();
	wdInit();
	watchdog_pause();
	wdPause();
	pfSampInit(10000);
	printf("CCR: %08x\n", *(int*)0xFF00001c);
	printf("RD: %i\n", sizeof(RasterData));
	fflush(stdout);
	//~ while(1);
#if !SKIPLOADER
	extern void run_loader();
	run_loader();
	return 0;
#endif
	
	maple_device_t *contr = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
	cont_state_t *cond = (cont_state_t *)maple_dev_status(contr);
	printf("Funcs     %08x\n", contr->info.functions);
	printf("FuncData0 %08x\n", contr->info.function_data[0]);
	printf("FuncData1 %08x\n", contr->info.function_data[1]);
	printf("FuncData2 %08x\n", contr->info.function_data[2]);
	
	//~ Controller_1_Type |= 0x10;
	
	#define ROMDIR "/pc/home/alex/Emu/Genesis/"
	//~ int runresult = run(ROMDIR"tanglewd.bin");
	int runresult = run(ROMDIR"sonic1.gen");
	
	if (Has_SRAM_Changed()) {
		printf("SRAM changed\n");
	
		if (Save_SRAM()) {
			printf("***Saved SRAM***\n");
		}
	} else {
		printf("No need to save\n");
	}
	
	if (Use_SDL_Audio == 2) {
		AICA_CMDSTR_CHANNEL(tmp, cmd, chan);

		/* Stop stream */
		/* Channel 0 */
		cmd->cmd = AICA_CMD_CHAN;
		cmd->timestamp = 0;
		cmd->size = AICA_CMDSTR_CHANNEL_SIZE;
		cmd->cmd_id = 0;
		chan->cmd = AICA_CH_CMD_STOP;
		snd_sh4_to_aica(tmp, cmd->size);

		/* Channel 1 */
		cmd->cmd_id = 1;
		snd_sh4_to_aica(tmp, cmd->size);
	}
	
	printf("runresult: %i\n", runresult);
	printf("Last alloc %p\n", malloc(4));
	fflush(stdout);
	//End_All ();
	return 0;
}

struct BMFFont * emufont;

int run(const char *rom_filename)
{
	printf("Running rom: %s\n", rom_filename);
	fflush(stdout);
	if (Get_Rom((char *)rom_filename) == -1)
		return -1;
	
	
	printf("Name : \"%.48s\"\n", Game->Rom_Name);
	printf("NameW: \"%.48s\"\n", Game->Rom_Name_W);
	printf("Version: \"%.13s\"\n", Game->Version);
	fflush(stdout);
	
	Sonic_2_Auto_Render = 0;
	Sonic_SMPS_Z80_Hack = 0;
	Ristar_Hack = 0;
	Force_Screen_On_Hack = 0;
	Render_Point = 0;
	
	if (GLCurGame) {
		printf("%08x\n", GLCurGame->Hacks);
		if (GLCurGame->Hacks & (1<<GH_SONIC_Z80_HACK))
			Sonic_SMPS_Z80_Hack = 1;
		if (GLCurGame->Hacks & (1<<GH_RISTAR_HACK))
			Ristar_Hack = 1;
		if (GLCurGame->Hacks & (1<<GH_SCREEN_OFF_RENDER_HACK))
			Render_Point = 2;
		if (GLCurGame->Hacks & (1<<GH_SONIC2_AUTO_RENDER_HACK))
			Sonic_2_Auto_Render = 1;
	} else {
		if (strcmp(Game->Rom_Name, "SONIC THE HEDGEHOG 2 ") == 0) {
			Sonic_SMPS_Z80_Hack = 1;
			Sonic_2_Auto_Render = 1;
		} else if (strcmp(Game->Rom_Name, "SONIC THE HEDGEHOG 3 ") == 0) {
			Sonic_SMPS_Z80_Hack = 1;
		} else if (strcmp(Game->Rom_Name, "SONIC & KNUCKLES ") == 0) {
			Sonic_SMPS_Z80_Hack = 1;
		} else if (strcmp(Game->Rom_Name, "- RISTAR - ") == 0) {
			Ristar_Hack = 1;
		} else if (strcmp(Game->Rom_Name, "- RISTAR THE SHOOTING STAR - ") == 0) {
			Ristar_Hack = 1;
		} else if (strcmp(Game->Rom_Name_W, "SONIC 3D FLICKIES' ISLAND ") == 0) {
			Sonic_SMPS_Z80_Hack = 1;
		} else if (strcmp(Game->Rom_Name_W, "GUNSTAR HEROES ") == 0) {
			
		} else if (strcmp(Game->Rom_Name_W, "Landstalker ") == 0) {
			Render_Point = 2;
		} else if (strcmp(Game->Rom_Name_W, "Land Stalker ") == 0) {
			Render_Point = 2;
		}
	}
	
	printf("Sonic Z80: %i\n", Sonic_SMPS_Z80_Hack);
	printf("Sonic 2: %i\n", Sonic_2_Auto_Render);
	printf("Ristar: %i\n", Ristar_Hack);
	fflush(stdout);
	chdir("/rd/");
	emufont = BMFLoad("uroob.fnt", "vq");
	
	prof_start(PROF_TOTAL);
	while (is_gens_running ())
	{
		watchdog_pet();
		
		//PROFILE START HERE
		prof_frame_start();
		
		
	      if (update_SDL_events () == -1)
		      return 0;

	#ifdef GENS_DEBUG
	      if (Debug)		// DEBUG
		{
		  Update_Debug_Screen ();
		  Flip ();
		}
	      else
	#endif
	      if (Genesis_Started || _32X_Started || SegaCD_Started)
		{
		  if ((Active) && (!Paused))	// EMULATION
		    {
			    //~ spu_master_mixer(9, 1);
				if (fast_forward == 1)
				{
					Update_Emulation_One ();
				}
				else
				{
					Update_Emulation ();
				}
		    }
		  else			// EMULATION PAUSED
		    {
			    Update_Controllers();
		      Flip ();
		      //~ Sleep (100);
		    }
		}
	      else if (GYM_Playing)	// PLAY GYM
		{
		  Play_GYM ();
		  Update_Gens_Logo ();
		}
	      else if (Intro_Style == 1)	// GENS LOGO EFFECT
		{
		  Update_Gens_Logo ();
		  Sleep (5);
		}
	      else if (Intro_Style == 2)	// STRANGE EFFECT
		{
		  Update_Crazy_Effect ();
		  Sleep (10);
		}
	      else if (Intro_Style == 3)	// GENESIS BIOS
		{
		  Do_Genesis_Frame ();
		  Flip ();
		  Sleep (20);
		}
	      else			// BLANK SCREEN (MAX IDLE)
		{
		  //Clear_Back_Screen();
		  Flip ();
		  Sleep (200);
		}
		
		prof_frame_end();
		//
	}
	prof_end(PROF_TOTAL);
	prof_show();
	
	
	BMFFree(emufont);
	
	return 0;
}
